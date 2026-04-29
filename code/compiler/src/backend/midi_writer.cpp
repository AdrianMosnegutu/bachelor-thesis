#include "dsl/backend/midi_writer.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "dsl/ir/program.hpp"
#include "dsl/music/instrument.hpp"
#include "dsl/music/key_mode.hpp"
#include "dsl/music/pitch.hpp"

namespace dsl::backend {

namespace {

constexpr uint16_t kTicksPerQuarter = 480;

struct MidiEvent {
    uint32_t tick{};
    std::vector<uint8_t> data;
};

// ── Binary helpers ────────────────────────────────────────────────────────────

void write_u16_be(std::ofstream& out, const uint16_t v) {
    out.put(static_cast<char>(v >> 8 & 0xFF));
    out.put(static_cast<char>(v & 0xFF));
}

void write_u32_be(std::ofstream& out, const uint32_t v) {
    out.put(static_cast<char>(v >> 24 & 0xFF));
    out.put(static_cast<char>(v >> 16 & 0xFF));
    out.put(static_cast<char>(v >> 8 & 0xFF));
    out.put(static_cast<char>(v & 0xFF));
}

void write_vlq(std::ofstream& out, uint32_t v) {
    uint8_t buf[4];
    int n = 0;
    buf[n++] = v & 0x7F;
    v >>= 7;
    while (v) {
        buf[n++] = v & 0x7F | 0x80;
        v >>= 7;
    }
    for (int i = n - 1; i >= 0; --i) out.put(static_cast<char>(buf[i]));
}

// ── Event serialisation ───────────────────────────────────────────────────────

void write_track(std::ofstream& out, std::vector<MidiEvent>& events) {
    // Sort by tick then write as delta-time events
    std::ranges::stable_sort(events, [](const MidiEvent& a, const MidiEvent& b) { return a.tick < b.tick; });

    // Compute total byte count (needed for MTrk length field)
    std::vector<std::pair<uint32_t, const std::vector<uint8_t>*>> deltas;
    deltas.reserve(events.size());
    uint32_t prev_tick = 0;
    for (const auto& [tick, data] : events) {
        uint32_t delta = tick - prev_tick;
        deltas.emplace_back(delta, &data);
        prev_tick = tick;
    }

    // Calculate byte length of track chunk data
    auto vlq_size = [](const uint32_t v) -> uint32_t {
        if (v < 0x80) return 1;
        if (v < 0x4000) return 2;
        if (v < 0x200000) return 3;
        return 4;
    };

    uint32_t chunk_len = 0;
    for (const auto& [delta, data] : deltas) chunk_len += vlq_size(delta) + static_cast<uint32_t>(data->size());

    // Write MTrk header
    out.write("MTrk", 4);
    write_u32_be(out, chunk_len);

    // Write events
    for (const auto& [delta, data] : deltas) {
        write_vlq(out, delta);
        out.write(reinterpret_cast<const char*>(data->data()), static_cast<std::streamsize>(data->size()));
    }
}

// ── Tempo track (track 0) ─────────────────────────────────────────────────────

int key_sig_sharps_flats(const music::PitchClass& pc) {
    using P = music::Pitch;
    using A = music::Accidental;
    // Encode as a single int to use as map key: pitch_semitone * 3 + (accidental + 1)
    auto enc = [](P p, A a) { return static_cast<int>(p) * 3 + static_cast<int>(a) + 1; };
    static const std::unordered_map<int, int> table{
        {enc(P::C, A::Natural), 0},
        {enc(P::G, A::Natural), 1},
        {enc(P::D, A::Natural), 2},
        {enc(P::A, A::Natural), 3},
        {enc(P::E, A::Natural), 4},
        {enc(P::B, A::Natural), 5},
        {enc(P::F, A::Sharp), 6},
        {enc(P::C, A::Flat), -7},
        {enc(P::G, A::Flat), -6},
        {enc(P::D, A::Flat), -5},
        {enc(P::A, A::Flat), -4},
        {enc(P::E, A::Flat), -3},
        {enc(P::B, A::Flat), -2},
        {enc(P::F, A::Natural), -1},
    };
    const auto it = table.find(enc(pc.pitch, pc.accidental));
    return it != table.end() ? it->second : 0;
}

std::vector<MidiEvent> build_tempo_track(const ir::ProgramIR& prog) {
    std::vector<MidiEvent> events;

    // Tempo: FF 51 03 tt tt tt
    const uint32_t uspb = 60000000u / static_cast<uint32_t>(prog.tempo_bpm);
    events.push_back({0,
                      {0xFF,
                       0x51,
                       0x03,
                       static_cast<uint8_t>(uspb >> 16 & 0xFF),
                       static_cast<uint8_t>(uspb >> 8 & 0xFF),
                       static_cast<uint8_t>(uspb & 0xFF)}});

    // Time signature: FF 58 04 nn dd cc bb
    int log2_denom = 0;
    int d = prog.time_sig_denominator;
    while (d > 1) {
        d >>= 1;
        ++log2_denom;
    }
    events.push_back(
        {0,
         {0xFF, 0x58, 0x04, static_cast<uint8_t>(prog.time_sig_numerator), static_cast<uint8_t>(log2_denom), 24, 8}});

    // Key signature: FF 59 02 sf mi
    if (prog.key) {
        const int sf = key_sig_sharps_flats(prog.key->pitch_class);
        uint8_t mi = prog.key->mode == music::KeyMode::Minor ? 1 : 0;
        events.push_back({0, {0xFF, 0x59, 0x02, static_cast<uint8_t>(static_cast<int8_t>(sf)), mi}});
    }

    // End of track
    events.push_back({0, {0xFF, 0x2F, 0x00}});
    return events;
}

// ── DSL track → MIDI track ────────────────────────────────────────────────────

std::vector<MidiEvent> build_dsl_track(const ir::TrackIR& track, const uint8_t channel) {
    std::vector<MidiEvent> events;
    const bool is_drums = track.instrument == music::Instrument::Drums;
    const uint8_t ch = is_drums ? 9 : channel;

    // Program change at tick 0 (drums don't need one)
    if (!is_drums) {
        auto prog = static_cast<uint8_t>(track.instrument);
        events.push_back({0, {static_cast<uint8_t>(0xC0 | ch), prog}});
    }

    // Note events
    for (const auto& [midi_note, start_beat, duration_beats, velocity] : track.events) {
        const auto on_tick = static_cast<uint32_t>(start_beat * kTicksPerQuarter);
        const auto off_tick = static_cast<uint32_t>((start_beat + duration_beats) * kTicksPerQuarter);
        auto note = static_cast<uint8_t>(midi_note);
        auto vel = static_cast<uint8_t>(velocity);

        events.push_back({on_tick, {static_cast<uint8_t>(0x90 | ch), note, vel}});
        events.push_back({off_tick, {static_cast<uint8_t>(0x80 | ch), note, 0}});
    }

    // End of track
    uint32_t end_tick = 0;
    for (const auto& [tick, data] : events) end_tick = std::max(end_tick, tick);
    events.push_back({end_tick, {0xFF, 0x2F, 0x00}});

    return events;
}

}  // namespace

// ── Public entry point ────────────────────────────────────────────────────────

void MidiWriter::write(const ir::ProgramIR& program, const std::string& output_path) {
    std::ofstream out(output_path, std::ios::binary);
    if (!out) throw std::runtime_error("cannot open output file: " + output_path);

    const uint16_t ntrks = 1 + static_cast<uint16_t>(program.tracks.size());

    // MThd chunk
    out.write("MThd", 4);
    write_u32_be(out, 6);  // chunk length always 6
    write_u16_be(out, 1);  // format 1
    write_u16_be(out, ntrks);
    write_u16_be(out, kTicksPerQuarter);

    // Track 0: tempo/meta
    auto tempo_track = build_tempo_track(program);
    write_track(out, tempo_track);

    // Remaining tracks: DSL tracks
    uint8_t channel = 0;
    for (const auto& dsl_track : program.tracks) {
        if (dsl_track.instrument != music::Instrument::Drums && channel == 9) ++channel;  // skip reserved drum channel
        auto track_events = build_dsl_track(dsl_track, channel);
        write_track(out, track_events);
        if (dsl_track.instrument != music::Instrument::Drums) ++channel;
    }

    if (!out) throw std::runtime_error("write error: " + output_path);
}

}  // namespace dsl::backend
