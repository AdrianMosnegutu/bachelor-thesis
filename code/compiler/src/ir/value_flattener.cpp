#include "dsl/ir/value_flattener.hpp"

#include <variant>

#include "dsl/utils/overloaded.hpp"

namespace dsl::ir {

std::vector<NoteEvent> flatten_value(const Value& value, const double start_beat, const double duration_beats) {
    std::vector<NoteEvent> events;

    std::visit(
        utils::overloaded{[&](const NoteVal& note) { detail::flatten_note(note, start_beat, duration_beats, events); },
                          [&](const ChordVal& chord) { detail::flatten_chord(chord, start_beat, events); },
                          [&](const SeqVal& sequence) { detail::flatten_sequence(sequence, start_beat, events); },
                          [](const auto&) { /* Silence RestVal/other types */ }},
        value.kind);

    return events;
}

}  // namespace dsl::ir
