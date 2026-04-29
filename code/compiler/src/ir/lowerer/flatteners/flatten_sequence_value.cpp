#include "dsl/core/utils/overloaded.hpp"
#include "dsl/ir/lowerer/value_flattener.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir::detail {

namespace {

double get_duration(const Value& value) {
    return std::visit(utils::overloaded{
                          [](const NoteValue& note) -> double { return note.duration_beats; },
                          [](const RestValue& rest) -> double { return rest.duration_beats; },
                          [](const ChordValue& chord) -> double { return chord.duration_beats; },
                          [](const auto&) -> double { return 0.0; },
                      },
                      value.kind);
}

}  // namespace

void flatten_sequence_value(const SequenceValue& sequence, const double start, NoteEvents& events) {
    double cursor = start;
    for (const auto& item_ptr : sequence.items) {
        const double item_duration = get_duration(*item_ptr);
        auto flattened_item = flatten_value(*item_ptr, cursor, item_duration);

        events.insert(events.end(), flattened_item.begin(), flattened_item.end());
        cursor += item_duration;
    }
}

}  // namespace dsl::ir::detail
