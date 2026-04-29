#pragma once

#include "dsl/ir/program.hpp"
#include "dsl/ir/value.hpp"

namespace dsl::ir {

namespace detail {

void flatten_note(const NoteVal& note, double start, double duration_beats, NoteEvents& events);
void flatten_chord(const ChordVal& chord, double start, NoteEvents& events);
void flatten_sequence(const SeqVal& sequence, double start, NoteEvents& events);

}  // namespace detail

[[nodiscard]] NoteEvents flatten_value(const Value& value, double start_beat, double duration_beats);

}  // namespace dsl::ir
