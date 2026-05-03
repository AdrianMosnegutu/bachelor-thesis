#include "dsl/lowering/detail/value_flattener.hpp"

#include <variant>

#include "dsl/common/ir/note_event.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/common/utils/overloaded.hpp"

namespace dsl::lowering::detail {

namespace {

double get_duration(const ir::Value& value) {
    return std::visit(utils::overloaded{
                          [](const ir::NoteValue& note) -> double { return note.duration_beats; },
                          [](const ir::RestValue& rest) -> double { return rest.duration_beats; },
                          [](const ir::ChordValue& chord) -> double { return chord.duration_beats; },
                          [](const auto&) -> double { return 0.0; },
                      },
                      value.kind);
}

}  // namespace

void flatten_note_value(const ir::NoteValue& note,
                        const double start,
                        const double duration_beats,
                        ir::NoteEvents& events) {
    const bool should_override_duration = duration_beats != 1.0 || note.duration_beats == 1.0;
    const double final_duration = should_override_duration ? duration_beats : note.duration_beats;

    events.push_back({note.midi_note, start, final_duration, note.velocity});
}

void flatten_chord_value(const ir::ChordValue& chord, const double start, ir::NoteEvents& events) {
    for (const auto& [midi_note, duration_beats, velocity] : chord.notes) {
        events.emplace_back(midi_note, start, duration_beats, velocity);
    }
}

void flatten_sequence_value(const ir::SequenceValue& sequence, const double start, ir::NoteEvents& events) {
    double cursor = start;
    for (const auto& item_ptr : sequence.items) {
        const double item_duration = get_duration(*item_ptr);
        auto flattened_item = flatten_value(*item_ptr, cursor, item_duration);

        events.insert(events.end(), flattened_item.begin(), flattened_item.end());
        cursor += item_duration;
    }
}

std::vector<ir::NoteEvent> flatten_value(const ir::Value& value, const double start_beat, const double duration_beats) {
    std::vector<ir::NoteEvent> events;

    std::visit(utils::overloaded{
                   [&](const ir::NoteValue& kind) { flatten_note_value(kind, start_beat, duration_beats, events); },
                   [&](const ir::ChordValue& kind) { flatten_chord_value(kind, start_beat, events); },
                   [&](const ir::SequenceValue& kind) { flatten_sequence_value(kind, start_beat, events); },
                   [](const auto&) { /* Any other value types do not emit MIDI events. */ }},
               value.kind);

    return events;
}

}  // namespace dsl::lowering::detail
