#include "dsl/ir/detail/lowerer/value_flattener.hpp"

namespace dsl::ir::detail {

void flatten_note_value(const NoteValue& note, const double start, const double duration_beats, NoteEvents& events) {
    const bool should_override_duration = duration_beats != 1.0 || note.duration_beats == 1.0;
    const double final_duration = should_override_duration ? duration_beats : note.duration_beats;

    events.push_back({note.midi_note, start, final_duration, note.velocity});
}

}  // namespace dsl::ir::detail
