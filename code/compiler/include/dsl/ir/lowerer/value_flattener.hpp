#pragma once

#include "dsl/ir/program.hpp"
#include "dsl/ir/value.hpp"

namespace dsl::ir {

namespace detail {

void flatten_note_value(const NoteValue& note, double start, double duration_beats, NoteEvents& events);
void flatten_chord_value(const ChordValue& chord, double start, NoteEvents& events);
void flatten_sequence_value(const SequenceValue& sequence, double start, NoteEvents& events);

}  // namespace detail

[[nodiscard]] NoteEvents flatten_value(const Value& value, double start_beat, double duration_beats);

}  // namespace dsl::ir
