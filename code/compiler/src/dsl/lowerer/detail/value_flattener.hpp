#pragma once

#include "dsl/ir/note_event.hpp"
#include "dsl/ir/values.hpp"

namespace dsl::lowerer::detail {

void flatten_note_value(const ir::NoteValue& note, double start, double duration_beats, ir::NoteEvents& events);
void flatten_chord_value(const ir::ChordValue& chord, double start, ir::NoteEvents& events);
void flatten_sequence_value(const ir::SequenceValue& sequence, double start, ir::NoteEvents& events);

[[nodiscard]] ir::NoteEvents flatten_value(const ir::Value& value, double start_beat, double duration_beats);

}  // namespace dsl::lowerer::detail
