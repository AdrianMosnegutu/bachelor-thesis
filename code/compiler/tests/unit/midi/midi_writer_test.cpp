#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "dsl/common/ir/program.hpp"
#include "dsl/common/music/instrument.hpp"
#include "dsl/midi/write_midi.hpp"

namespace fs = std::filesystem;
namespace midi = dsl::midi;

using dsl::ir::Program;
using dsl::ir::Track;
using dsl::music::Instrument;

// -- Helpers -------------------------------------------------------------------

namespace {

std::vector<uint8_t> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return {std::istreambuf_iterator(f), {}};
}

uint16_t read_u16_be(const std::vector<uint8_t>& data, const size_t offset) {
    return static_cast<uint16_t>(data[offset] << 8 | data[offset + 1]);
}

uint32_t read_u32_be(const std::vector<uint8_t>& data, const size_t offset) {
    return static_cast<uint32_t>(data[offset]) << 24 | static_cast<uint32_t>(data[offset + 1]) << 16 |
           static_cast<uint32_t>(data[offset + 2]) << 8 | static_cast<uint32_t>(data[offset + 3]);
}

// RAII temp file
struct TempFile {
    std::string path;
    explicit TempFile(const std::string& name) : path((fs::temp_directory_path() / name).string()) {}
    ~TempFile() { fs::remove(path); }
};

Program make_single_note_program() {
    Program prog;
    prog.tempo_bpm = 120;
    prog.time_sig_numerator = 4;
    prog.time_sig_denominator = 4;

    Track track;
    track.instrument = Instrument::Piano;
    track.events.push_back({60, 0.0, 1.0, 100});  // middle C, beat 0, 1 beat
    prog.tracks.push_back(std::move(track));
    return prog;
}

}  // namespace

// -- Tests ---------------------------------------------------------------------

TEST(MidiWriter, WritesMThd) {
    TempFile tmp("test_mthd.mid");
    midi::write_midi(make_single_note_program(), tmp.path);

    auto data = read_file(tmp.path);
    ASSERT_GE(data.size(), 14u);

    // Magic
    EXPECT_EQ(data[0], 'M');
    EXPECT_EQ(data[1], 'T');
    EXPECT_EQ(data[2], 'h');
    EXPECT_EQ(data[3], 'd');

    // Chunk length = 6
    EXPECT_EQ(read_u32_be(data, 4), 6u);

    // Format = 1
    EXPECT_EQ(read_u16_be(data, 8), 1u);

    // ntrks = 1 (tempo) + 1 (DSL track)
    EXPECT_EQ(read_u16_be(data, 10), 2u);

    // Division = 480
    EXPECT_EQ(read_u16_be(data, 12), 480u);
}

TEST(MidiWriter, MultipleTracksProduceCorrectNtrks) {
    const TempFile tmp("test_ntrks.mid");
    Program prog;
    prog.tracks.push_back(Track{});
    prog.tracks.push_back(Track{});
    prog.tracks.push_back(Track{});

    midi::write_midi(prog, tmp.path);

    const auto data = read_file(tmp.path);
    ASSERT_GE(data.size(), 14u);
    EXPECT_EQ(read_u16_be(data, 10), 4u);  // 1 tempo + 3 DSL
}

TEST(MidiWriter, WritesSingleNote) {
    TempFile tmp("test_single_note.mid");
    midi::write_midi(make_single_note_program(), tmp.path);

    auto data = read_file(tmp.path);

    // File must start with MIDI magic
    ASSERT_GE(data.size(), 4u);
    EXPECT_EQ(data[0], 0x4D);  // 'M'
    EXPECT_EQ(data[1], 0x54);  // 'T'
    EXPECT_EQ(data[2], 0x68);  // 'h'
    EXPECT_EQ(data[3], 0x64);  // 'd'

    // Second chunk must be a tempo MTrk
    EXPECT_EQ(data[14], 'M');
    EXPECT_EQ(data[15], 'T');
    EXPECT_EQ(data[16], 'r');
    EXPECT_EQ(data[17], 'k');
}

TEST(MidiWriter, TempoTrackContainsTempoEvent) {
    TempFile tmp("test_tempo.mid");
    Program prog;
    prog.tempo_bpm = 120;
    prog.time_sig_numerator = 4;
    prog.time_sig_denominator = 4;
    prog.tracks.push_back(Track{});

    midi::write_midi(prog, tmp.path);

    auto data = read_file(tmp.path);

    // Tempo track starts at byte 14. Skip MTrk header (8 bytes).
    // First event delta=0 (1 byte), then FF 51 03 ...
    ASSERT_GE(data.size(), 14u + 8u + 5u);
    size_t pos = 14 + 8;             // start of tempo track data
    EXPECT_EQ(data[pos], 0x00);      // delta = 0
    EXPECT_EQ(data[pos + 1], 0xFF);  // meta event
    EXPECT_EQ(data[pos + 2], 0x51);  // tempo type
    EXPECT_EQ(data[pos + 3], 0x03);  // length = 3
    // microseconds/beat = 60000000/120 = 500000 = 0x07A120
    EXPECT_EQ(data[pos + 4], 0x07);
    EXPECT_EQ(data[pos + 5], 0xA1);
    EXPECT_EQ(data[pos + 6], 0x20);
}

TEST(MidiWriter, DrumTrackUsesChannel9) {
    const TempFile tmp("test_drums.mid");
    Program prog;
    prog.tempo_bpm = 120;
    prog.time_sig_numerator = 4;
    prog.time_sig_denominator = 4;

    Track drums;
    drums.instrument = Instrument::Drums;
    drums.events.push_back({36, 0.0, 0.5, 100});  // kick drum
    prog.tracks.push_back(std::move(drums));

    midi::write_midi(prog, tmp.path);

    const auto data = read_file(tmp.path);
    // Search for a note-on event on channel 9 (0x99) with note 36
    bool found = false;
    for (size_t i = 0; i + 2 < data.size(); ++i) {
        if (data[i] == 0x99 && data[i + 1] == 36) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found) << "Note-on on channel 9 not found for drum track";
}

TEST(MidiWriter, NoDrumProgramChange) {
    const TempFile tmp("test_no_drum_pc.mid");
    Program prog;
    prog.tempo_bpm = 120;
    prog.time_sig_numerator = 4;
    prog.time_sig_denominator = 4;

    Track drums;
    drums.instrument = Instrument::Drums;
    drums.events.push_back({36, 0.0, 0.5, 100});
    prog.tracks.push_back(std::move(drums));

    midi::write_midi(prog, tmp.path);

    const auto data = read_file(tmp.path);
    // Program change on channel 9 would be 0xC9 — must not appear
    bool found = false;
    for (const unsigned char i : data) {
        if (i == 0xC9) {
            found = true;
            break;
        }
    }
    EXPECT_FALSE(found) << "Unexpected program change event on drum channel";
}

TEST(MidiWriter, ThrowsOnBadOutputPath) {
    Program prog;
    EXPECT_THROW(midi::write_midi(prog, "/nonexistent_dir/out.mid"), std::runtime_error);
}

TEST(MidiWriter, EmptyProgramWritesValidFile) {
    TempFile tmp("test_empty.mid");
    Program prog;
    EXPECT_NO_THROW(midi::write_midi(prog, tmp.path));

    auto data = read_file(tmp.path);
    ASSERT_GE(data.size(), 14u);
    EXPECT_EQ(data[0], 'M');
    EXPECT_EQ(data[1], 'T');
    EXPECT_EQ(data[2], 'h');
    EXPECT_EQ(data[3], 'd');
    EXPECT_EQ(read_u16_be(data, 10), 1u);  // only tempo track
}
