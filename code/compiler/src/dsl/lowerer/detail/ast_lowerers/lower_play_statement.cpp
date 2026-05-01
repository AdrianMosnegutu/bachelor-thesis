#include "dsl/common/ast/statements.hpp"
#include "dsl/common/ir/note_event.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/common/utils/overloaded.hpp"
#include "dsl/lowerer/detail/expression_evaluator.hpp"
#include "dsl/lowerer/detail/lowerer_context.hpp"
#include "dsl/lowerer/detail/value_flattener.hpp"

namespace dsl::lowerer::detail {

namespace {

using ir::ChordValue;
using ir::NoteEvents;
using ir::NoteValue;
using ir::RestValue;
using ir::SequenceValue;
using ir::Value;

}  // namespace

NoteEvents lower_play_statement(const ast::PlayStatement& play_stmt, LowererContext& ctx, double& cursor) {
    const auto& target = play_stmt.target;

    // Resolve duration: explicit :dur overrides default of 1 beat.
    double stmt_duration = 1.0;
    if (target.duration) {
        auto [kind] = evaluate_expression(*target.duration, ctx);

        stmt_duration = std::visit(
            utils::overloaded{[](const int number) { return static_cast<double>(number); },
                              [](const double number) { return number; },
                              [&](const auto&) -> double {
                                  throw LoweringFailure(play_stmt.target.location,
                                                        "lowering reached play statement with non-numeric duration");
                              }},
            kind);
    }

    // Resolve start beat.
    double start;
    bool has_from = target.from_offset != nullptr;
    if (!has_from) {
        start = cursor;
    } else {
        auto [kind] = evaluate_expression(*target.from_offset, ctx);

        start = std::visit(utils::overloaded{[](const int number) { return static_cast<double>(number); },
                                             [](const double number) { return number; },
                                             [&](const auto&) -> double {
                                                 throw LoweringFailure(
                                                     play_stmt.target.location,
                                                     "lowering reached play statement with non-numeric from offset");
                                             }},
                           kind);
    }

    // Evaluate the play source.
    Value val = std::visit(
        utils::overloaded{
            [&ctx](const ast::ExpressionPtr& ptr) { return evaluate_expression(*ptr, ctx); },
            [&](const music::DrumNote& drum) { return Value{NoteValue{static_cast<int>(drum), stmt_duration, 100}}; }},
        target.source);

    // For chords: if no explicit :dur, let each note keep its own duration and
    // advance the cursor by the chord's max duration. If :dur is explicit, apply
    // it uniformly to all notes.
    if (auto* cv = std::get_if<ChordValue>(&val.kind)) {
        if (target.duration) {
            for (auto& note : cv->notes) {
                note.duration_beats = stmt_duration;
            }
            cv->duration_beats = stmt_duration;
        }
    }

    auto events = flatten_value(val, start, stmt_duration);

    if (!has_from) {
        double total = 0.0;
        if (const auto* sv = std::get_if<SequenceValue>(&val.kind)) {
            for (const auto& item_ptr : sv->items) {
                total += std::visit(utils::overloaded{
                                        [](const NoteValue& n) { return n.duration_beats; },
                                        [](const RestValue& r) { return r.duration_beats; },
                                        [](const ChordValue& c) { return c.duration_beats; },
                                        [](const auto&) { return 0.0; },
                                    },
                                    item_ptr->kind);
            }
        } else if (const auto* cv = std::get_if<ChordValue>(&val.kind)) {
            total = cv->duration_beats;
        } else {
            total = stmt_duration;
        }
        cursor += total;
    }

    return events;
}

}  // namespace dsl::lowerer::detail
