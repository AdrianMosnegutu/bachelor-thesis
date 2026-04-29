#include "dsl/ir/lowerer/value_flattener.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir::detail {

void flatten_chord_value(const ChordValue& chord, const double start, NoteEvents& events) {
    for (const auto& [midi_note, duration_beats, velocity] : chord.notes) {
        events.emplace_back(midi_note, start, duration_beats, velocity);
    }
}

}  // namespace dsl::ir::detail
