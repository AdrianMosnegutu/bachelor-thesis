#include "dsl/core/utils/overloaded.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/ir/value_flattener.hpp"

namespace dsl::ir::detail {

namespace {

double get_duration(const Value& value) {
    return std::visit(utils::overloaded{
                          [](const NoteVal& note) -> double { return note.duration_beats; },
                          [](const RestVal& rest) -> double { return rest.duration_beats; },
                          [](const ChordVal& chord) -> double { return chord.duration_beats; },
                          [](const auto&) -> double { return 0.0; },
                      },
                      value.kind);
}

}  // namespace

void flatten_sequence(const SeqVal& sequence, const double start, NoteEvents& events) {
    double cursor = start;
    for (const auto& item_ptr : sequence.items) {
        const double item_duration = get_duration(*item_ptr);
        auto flattened_item = flatten_value(*item_ptr, cursor, item_duration);

        events.insert(events.end(), flattened_item.begin(), flattened_item.end());
        cursor += item_duration;
    }
}

}  // namespace dsl::ir::detail
