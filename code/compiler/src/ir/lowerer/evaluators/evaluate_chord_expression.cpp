#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/ir/lowerer/expression_evaluator.hpp"

namespace dsl::ir::detail {

using errors::SemanticError;

Value evaluate_chord_expression(const ast::ChordExpression& chord, const Location& loc, LowererContext& context) {
    ChordValue chord_value;
    double max_duration = 0.0;

    for (const auto& [value, duration] : chord.notes) {
        auto [kind] = evaluate_expression(*value, context);
        auto* note_value = std::get_if<NoteValue>(&kind);

        if (!note_value) {
            throw SemanticError(loc, "chord members must be notes");
        }

        if (duration) {
            auto [duration_kind] = evaluate_expression(*duration, context);

            if (auto* integer = std::get_if<int>(&duration_kind)) {
                note_value->duration_beats = static_cast<double>(*integer);
            } else if (auto* floating_point = std::get_if<double>(&duration_kind)) {
                note_value->duration_beats = *floating_point;
            } else {
                throw SemanticError(duration->location, "chord note duration must be numeric");
            }
        }

        max_duration = std::max(max_duration, note_value->duration_beats);
        chord_value.notes.push_back(*note_value);
    }

    chord_value.duration_beats = max_duration > 0.0 ? max_duration : 1.0;
    return Value{std::move(chord_value)};
}

}  // namespace dsl::ir::detail
