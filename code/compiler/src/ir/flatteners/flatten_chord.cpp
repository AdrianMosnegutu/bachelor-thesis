#include "dsl/ir/program.hpp"
#include "dsl/ir/value_flattener.hpp"

namespace dsl::ir::detail {

void flatten_chord(const ChordVal& chord, const double start, NoteEvents& events) {
    for (const auto& [midi_note, duration_beats, velocity] : chord.notes) {
        events.emplace_back(midi_note, start, duration_beats, velocity);
    }
}

}  // namespace dsl::ir::detail
