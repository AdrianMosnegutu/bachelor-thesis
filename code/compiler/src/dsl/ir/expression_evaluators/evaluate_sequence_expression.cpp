#include "dsl/errors/lowerer_error.hpp"
#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir {

Value evaluate_sequence_expression(const ast::SequenceExpression& sequence, LowererContext& context) {
    SequenceValue sequence_val;

    for (const auto& [value, duration] : sequence.items) {
        Value evaluated_value = evaluate_expression(*value, context);

        if (duration) {
            auto [kind] = evaluate_expression(*duration, context);
            double raw_duration;

            if (auto* integer = std::get_if<int>(&kind)) {
                raw_duration = static_cast<double>(*integer);
            } else if (auto* floating_point = std::get_if<double>(&kind)) {
                raw_duration = *floating_point;
            } else {
                throw errors::LowererError(duration->location,
                                           "lowering reached sequence item with non-numeric duration");
            }

            if (auto* note = std::get_if<NoteValue>(&evaluated_value.kind)) {
                note->duration_beats = raw_duration;
            } else if (auto* rest = std::get_if<RestValue>(&evaluated_value.kind)) {
                rest->duration_beats = raw_duration;
            }
        }

        sequence_val.items.push_back(std::make_shared<Value>(std::move(evaluated_value)));
    }

    return Value{std::move(sequence_val)};
}

}  // namespace dsl::ir
