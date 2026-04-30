#pragma once

#include <vector>

namespace dsl::ir {

struct NoteEvent {
    int midi_note{};          // MIDI note number (0–127)
    double start_beat{};      // absolute beat position from track start
    double duration_beats{};  // note duration in beats
    int velocity{100};        // MIDI velocity
};

using NoteEvents = std::vector<NoteEvent>;

}  // namespace dsl::ir