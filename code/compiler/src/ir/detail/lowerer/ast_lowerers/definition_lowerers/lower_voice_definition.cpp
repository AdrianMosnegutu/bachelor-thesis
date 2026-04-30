#include "dsl/core/errors/lowerer_error.hpp"
#include "dsl/core/utils/overloaded.hpp"
#include "dsl/ir/detail/expression_evaluator.hpp"
#include "dsl/ir/detail/lowerer/ast_lowerers.h"

namespace dsl::ir::detail {

NoteEvents lower_voice_definition(const ast::VoiceDefinition& voice, LowererContext& ctx, const double outer_cursor) {
    double voice_cursor = outer_cursor;

    if (voice.from_expression) {
        const auto [kind] = evaluate_expression(**voice.from_expression, ctx);

        voice_cursor = std::visit(
            utils::overloaded{[](const int number) { return static_cast<double>(number); },
                              [](const double number) { return number; },
                              [&voice](const auto&) -> double {
                                  throw errors::LowererError(voice.location,
                                                             "lowering reached voice with non-numeric from expression");
                              }},
            kind);
    }

    ctx.collect_voice_patterns(voice.body);
    ctx.push_scope();

    std::vector<NoteEvent> events;
    for (const auto& item : voice.body) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            auto evs = lower_statement(**stmt_ptr, ctx, voice_cursor);
            events.insert(events.end(), evs.begin(), evs.end());
        }
        // PatternDefinition items already registered — skip execution.
    }

    ctx.pop_scope();
    ctx.erase_voice_patterns(voice.body);

    // outer_cursor is intentionally not modified — voice runs in parallel.
    return events;
}

}  // namespace dsl::ir::detail
