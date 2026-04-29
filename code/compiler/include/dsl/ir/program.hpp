#pragma once

#include <optional>
#include <string>
#include <vector>

#include "dsl/core/music/instrument.hpp"

namespace dsl::ir {

struct NoteEvent {
    int midi_note{};          // MIDI note number (0–127)
    double start_beat{};      // absolute beat position from track start
    double duration_beats{};  // note duration in beats
    int velocity{100};        // MIDI velocity
};

struct TrackIR {
    std::optional<std::string> name;
    music::Instrument instrument{music::Instrument::Piano};
    std::vector<NoteEvent> events;  // sorted by start_beat after lowering
};

struct ProgramIR {
    int tempo_bpm{120};
    int time_sig_numerator{4};
    int time_sig_denominator{4};
    std::vector<TrackIR> tracks;
};

using NoteEvents = std::vector<NoteEvent>;

}  // namespace dsl::ir
