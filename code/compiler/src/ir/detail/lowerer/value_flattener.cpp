#include "dsl/ir/detail/lowerer/value_flattener.hpp"

#include <variant>

#include "dsl/core/utils/overloaded.hpp"

namespace dsl::ir::detail {

std::vector<NoteEvent> flatten_value(const Value& value, const double start_beat, const double duration_beats) {
    std::vector<NoteEvent> events;

    std::visit(
        utils::overloaded{[&](const NoteValue& kind) { flatten_note_value(kind, start_beat, duration_beats, events); },
                          [&](const ChordValue& kind) { flatten_chord_value(kind, start_beat, events); },
                          [&](const SequenceValue& kind) { flatten_sequence_value(kind, start_beat, events); },
                          [](const auto&) { /* Any other value types do not emit MIDI events. */ }},
        value.kind);

    return events;
}

}  // namespace dsl::ir::detail
