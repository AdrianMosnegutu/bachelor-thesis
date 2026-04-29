#include "dsl/core/ast/statements/play_statement.h"
#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/core/utils/overloaded.hpp"
#include "dsl/ir/expression_evaluator.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/value_flattener.hpp"

namespace dsl::ir {

using errors::SemanticError;

std::vector<NoteEvent> Lowerer::lower_play(const ast::PlayStatement& stmt, LowererContext& ctx, double& cursor) {
    const auto& target = stmt.target;

    // Resolve duration: explicit :dur overrides default of 1 beat.
    double dur = 1.0;
    if (target.duration) {
        auto [kind] = evaluate_expression(*target.duration, ctx);
        if (const auto* i = std::get_if<int>(&kind)) {
            dur = static_cast<double>(*i);
        } else if (const auto* d = std::get_if<double>(&kind)) {
            dur = *d;
        } else {
            throw SemanticError(target.duration->location, "play duration must be a number");
        }
    }

    // Resolve start beat.
    double start;
    bool has_from = target.from_offset != nullptr;
    if (has_from) {
        auto [kind] = evaluate_expression(*target.from_offset, ctx);
        if (const auto* i = std::get_if<int>(&kind)) {
            start = static_cast<double>(*i);
        } else if (const auto* d = std::get_if<double>(&kind)) {
            start = *d;
        } else {
            throw SemanticError(target.from_offset->location, "from offset must be a number");
        }
    } else {
        start = cursor;
    }

    // Evaluate the play source.
    Value val;
    if (const auto* expr_src = std::get_if<ast::ExpressionPtr>(&target.source)) {
        val = evaluate_expression(**expr_src, ctx);
    } else {
        const auto& drum = std::get<music::DrumNote>(target.source);
        val = Value{NoteVal{static_cast<int>(drum), dur, 100}};
    }

    // For chords: if no explicit :dur, let each note keep its own duration and
    // advance the cursor by the chord's max duration. If :dur is explicit, apply
    // it uniformly to all notes.
    if (auto* cv = std::get_if<ChordVal>(&val.kind)) {
        if (target.duration) {
            for (auto& note : cv->notes) note.duration_beats = dur;
            cv->duration_beats = dur;
        }
    }

    auto events = flatten_value(val, start, dur);

    if (!has_from) {
        double total = 0.0;
        if (const auto* sv = std::get_if<SeqVal>(&val.kind)) {
            for (const auto& item_ptr : sv->items) {
                total += std::visit(utils::overloaded{
                                        [](const NoteVal& n) -> double { return n.duration_beats; },
                                        [](const RestVal& r) -> double { return r.duration_beats; },
                                        [](const ChordVal& c) -> double { return c.duration_beats; },
                                        [](const auto&) -> double { return 0.0; },
                                    },
                                    item_ptr->kind);
            }
        } else if (const auto* cv = std::get_if<ChordVal>(&val.kind)) {
            total = cv->duration_beats;
        } else {
            total = dur;
        }
        cursor += total;
    }

    return events;
}

}  // namespace dsl::ir
