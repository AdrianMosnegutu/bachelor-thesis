#include "dsl/ast/expressions.hpp"
#include "dsl/errors/lowerer_error.hpp"
#include "dsl/ir/values.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"
#include "dsl/lowerer/detail/lowerer_context.hpp"

namespace dsl::lowerer::detail {

namespace {

using ir::ChordValue;
using ir::NoteValue;
using ir::RestValue;
using ir::SequenceValue;
using ir::Value;
using ir::ValueKind;

}  // namespace

Value evaluate_pattern_call_expression(const ast::PatternCallExpression& call,
                                       const source::Location& loc,
                                       LowererContext& context) {
    const auto* pattern = context.find_pattern(call.callee);
    if (!pattern) {
        throw errors::LowererError(loc, "lowering reached unresolved pattern '" + call.callee + "'");
    }

    if (call.arguments.size() != pattern->params.size()) {
        throw errors::LowererError(loc, "lowering reached pattern '" + call.callee + "' with invalid arity");
    }

    // Evaluate all arguments in the caller's scope before opening the callee's
    // scope. Doing it inside push_scope() would let each successive bind shadow
    // identifiers that share a name with earlier callee params.
    std::vector<Value> arg_vals;
    arg_vals.reserve(call.arguments.size());
    for (const auto& arg : call.arguments) {
        arg_vals.push_back(evaluate_expression(*arg, context));
    }

    context.push_scope();
    for (std::size_t i = 0; i < pattern->params.size(); ++i) {
        context.bind(pattern->params[i], std::move(arg_vals[i]));
    }

    double inner_cursor = 0.0;
    const auto events = context.execute_block(pattern->body, inner_cursor);
    context.pop_scope();

    // Pure-rest pattern: no note events but cursor advanced (all rests).
    if (events.empty()) {
        if (inner_cursor > 1e-9) {
            SequenceValue rest_only;
            rest_only.items.push_back(std::make_shared<Value>(Value{RestValue{inner_cursor}}));
            return Value{std::move(rest_only)};
        }
        return Value{SequenceValue{}};
    }

    // Rebuild a SeqVal that preserves the full temporal structure of the pattern,
    // including rests. We walk event groups in beat order, inserting RestVal items
    // for every gap between consecutive groups and for any trailing silence.
    SequenceValue sequence_value;
    double cursor = 0.0;
    std::size_t event_i = 0;

    while (event_i < events.size()) {
        const double beat = events[event_i].start_beat;

        // Gap before this event group → rest.
        if (beat - cursor > 1e-9) {
            sequence_value.items.push_back(std::make_shared<Value>(Value{RestValue{beat - cursor}}));
        }

        // Find the extent of this beat group (simultaneous events = chord).
        std::size_t event_j = event_i;
        while (event_j < events.size() && events[event_j].start_beat == beat) {
            ++event_j;
        }

        double group_duration = 0.0;

        if (event_j - event_i == 1) {
            const auto& event = events[event_i];
            group_duration = event.duration_beats;
            sequence_value.items.push_back(
                std::make_shared<Value>(Value{NoteValue{event.midi_note, event.duration_beats, event.velocity}}));
        } else {
            ChordValue chord;
            chord.duration_beats = 0.0;

            for (std::size_t k = event_i; k < event_j; ++k) {
                const auto& event = events[k];
                chord.notes.push_back(NoteValue{event.midi_note, event.duration_beats, event.velocity});
                chord.duration_beats = std::max(chord.duration_beats, event.duration_beats);
            }

            if (chord.duration_beats == 0.0) {
                chord.duration_beats = 1.0;
            }

            group_duration = chord.duration_beats;
            sequence_value.items.push_back(std::make_shared<Value>(Value{std::move(chord)}));
        }

        cursor = beat + group_duration;
        event_i = event_j;
    }

    // Trailing silence: pattern body advanced the cursor past the last note.
    if (inner_cursor - cursor > 1e-9) {
        sequence_value.items.push_back(std::make_shared<Value>(Value{RestValue{inner_cursor - cursor}}));
    }

    if (sequence_value.items.size() == 1) {
        return *sequence_value.items[0];
    }

    return Value{std::move(sequence_value)};
}

}  // namespace dsl::lowerer::detail
