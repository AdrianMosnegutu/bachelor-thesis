#include <string>

#include "dsl/common/ast/statements.hpp"
#include "dsl/common/ir/note_event.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/common/utils/overloaded.hpp"
#include "dsl/lowering/detail/expression_evaluator.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"
#include "dsl/lowering/detail/value_flattener.hpp"

namespace dsl::lowering::detail {

namespace {

using ir::ChordValue;
using ir::NoteEvents;
using ir::NoteValue;
using ir::RestValue;
using ir::SequenceValue;
using ir::Value;

double chord_cursor_advance(const ChordValue& chord) {
    if (chord.duration_beats != 0.0) {
        return chord.duration_beats;
    }
    double max_dur = 1.0;
    for (const auto& note : chord.notes) {
        max_dur = std::max(max_dur, note.duration_beats);
    }
    return max_dur;
}

double as_beats(const ir::ValueKind& kind) {
    if (const auto* integer = std::get_if<int>(&kind)) {
        return *integer;
    }

    return std::get<double>(kind);
}

}  // namespace

NoteEvents lower_play_statement(const ast::PlayStatement& play_stmt, LowererContext& ctx, double& cursor) {
    const auto& target = play_stmt.target;

    // Resolve duration: explicit :dur overrides default of 1 beat.
    double stmt_duration = 1.0;
    if (target.duration) {
        stmt_duration = as_beats(evaluate_expression(*target.duration, ctx).kind);
    }

    // Resolve start beat.
    const bool has_from = target.from_offset != nullptr;
    const double start = has_from ? as_beats(evaluate_expression(*target.from_offset, ctx).kind) : cursor;

    if (has_from && start < 0.0) {
        throw LoweringFailure(target.from_offset->location,
                              "negative from offset (" + std::to_string(start) + ")");
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
    ctx.register_events(events.size(), target.location);

    if (!has_from) {
        double total = 0.0;
        if (const auto* sv = std::get_if<SequenceValue>(&val.kind)) {
            for (const auto& item_ptr : sv->items) {
                total += std::visit(utils::overloaded{
                                        [](const NoteValue& n) { return n.duration_beats; },
                                        [](const RestValue& r) { return r.duration_beats; },
                                        [](const ChordValue& c) { return chord_cursor_advance(c); },
                                        [](const auto&) { return 0.0; },
                                    },
                                    item_ptr->kind);
            }
        } else if (const auto* cv = std::get_if<ChordValue>(&val.kind)) {
            total = chord_cursor_advance(*cv);
        } else if (const auto* nv = std::get_if<NoteValue>(&val.kind)) {
            // Mirror flatten_note_value: explicit :dur overrides the note's own duration;
            // otherwise respect the note's embedded duration (e.g. from a pattern return).
            const bool override = stmt_duration != 1.0 || nv->duration_beats == 1.0;
            total = override ? stmt_duration : nv->duration_beats;
        } else if (const auto* rv = std::get_if<RestValue>(&val.kind)) {
            const bool override = stmt_duration != 1.0 || rv->duration_beats == 1.0;
            total = override ? stmt_duration : rv->duration_beats;
        } else {
            total = stmt_duration;
        }
        cursor += total;
    }

    return events;
}

}  // namespace dsl::lowering::detail
