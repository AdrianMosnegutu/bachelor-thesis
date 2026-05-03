#include "dsl/common/ir/values.hpp"
#include "dsl/lowering/detail/expression_evaluator.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"

namespace dsl::lowering::detail {

namespace {

using ir::ChordValue;
using ir::SequenceValue;
using ir::Value;

}  // namespace

Value evaluate_chord_expression(const ast::ChordExpression& chord,
                                const source::Location& loc,
                                LowererContext& context) {
    ChordValue chord_value;
    double max_duration = 0.0;

    for (const auto& [value, duration] : chord.notes) {
        auto [kind] = evaluate_expression(*value, context);
        auto* note_value = std::get_if<ir::NoteValue>(&kind);

        if (!note_value) {
            throw LoweringFailure(loc, "lowering reached chord with a non-note member");
        }

        if (duration) {
            auto [duration_kind] = evaluate_expression(*duration, context);

            if (auto* integer = std::get_if<int>(&duration_kind)) {
                note_value->duration_beats = static_cast<double>(*integer);
            } else if (auto* floating_point = std::get_if<double>(&duration_kind)) {
                note_value->duration_beats = *floating_point;
            } else {
                throw LoweringFailure(loc, "lowering reached chord note with non-numeric duration");
            }
        }

        max_duration = std::max(max_duration, note_value->duration_beats);
        chord_value.notes.push_back(*note_value);
    }

    chord_value.duration_beats = max_duration > 0.0 ? max_duration : 1.0;
    return Value{std::move(chord_value)};
}

Value evaluate_sequence_expression(const ast::SequenceExpression& sequence, LowererContext& context) {
    SequenceValue sequence_val;

    for (const auto& [value, duration] : sequence.items) {
        Value evaluated_value = evaluate_expression(*value, context);

        if (duration) {
            auto [kind] = evaluate_expression(*duration, context);
            double raw_duration;

            if (const auto* integer = std::get_if<int>(&kind)) {
                raw_duration = static_cast<double>(*integer);
            } else if (const auto* floating_point = std::get_if<double>(&kind)) {
                raw_duration = *floating_point;
            } else {
                throw LoweringFailure(duration->location, "lowering reached sequence item with non-numeric duration");
            }

            if (auto* note = std::get_if<ir::NoteValue>(&evaluated_value.kind)) {
                note->duration_beats = raw_duration;
            } else if (auto* rest = std::get_if<ir::RestValue>(&evaluated_value.kind)) {
                rest->duration_beats = raw_duration;
            }
        }

        sequence_val.items.push_back(std::make_shared<Value>(std::move(evaluated_value)));
    }

    return Value{std::move(sequence_val)};
}

}  // namespace dsl::lowering::detail
