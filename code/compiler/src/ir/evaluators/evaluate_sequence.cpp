#include "dsl/errors/semantic_error.hpp"
#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir::detail {

using errors::SemanticError;

Value evaluate_sequence(const ast::Sequence& sequence, LowererContext& context) {
    SeqVal sequence_val;

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
                throw SemanticError(duration->loc, "sequence item duration must be numeric");
            }

            if (auto* note = std::get_if<NoteVal>(&evaluated_value.kind)) {
                note->duration_beats = raw_duration;
            } else if (auto* rest = std::get_if<RestVal>(&evaluated_value.kind)) {
                rest->duration_beats = raw_duration;
            }
        }

        sequence_val.items.push_back(std::make_shared<Value>(std::move(evaluated_value)));
    }

    return Value{std::move(sequence_val)};
}

}  // namespace dsl::ir::detail
