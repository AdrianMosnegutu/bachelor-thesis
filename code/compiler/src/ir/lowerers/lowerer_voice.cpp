#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/ir/expression_evaluator.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/lowerer_context.hpp"

namespace dsl::ir {

using errors::SemanticError;

std::vector<NoteEvent> Lowerer::lower_voice(const ast::VoiceDefinition& voice,
                                            LowererContext& ctx,
                                            double outer_cursor) {
    double voice_cursor = outer_cursor;
    if (voice.from_expression) {
        const auto val = evaluate_expression(**voice.from_expression, ctx);
        if (const auto* d = std::get_if<double>(&val.kind)) {
            voice_cursor = *d;
        } else if (const auto* i = std::get_if<int>(&val.kind)) {
            voice_cursor = static_cast<double>(*i);
        } else {
            throw SemanticError(voice.location, "voice 'from' expression must be numeric");
        }
    }

    ctx.collect_voice_patterns(voice.body);
    ctx.push_scope();

    std::vector<NoteEvent> events;
    for (const auto& item : voice.body) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            auto evs = lower_stmt(**stmt_ptr, ctx, voice_cursor);
            events.insert(events.end(), evs.begin(), evs.end());
        }
        // PatternDefinition items already registered — skip execution.
    }

    ctx.pop_scope();
    ctx.erase_voice_patterns(voice.body);

    // outer_cursor is intentionally not modified — voice runs in parallel.
    return events;
}

}  // namespace dsl::ir
