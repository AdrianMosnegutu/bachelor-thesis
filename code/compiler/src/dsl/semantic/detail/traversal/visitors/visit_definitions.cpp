#include "dsl/common/utils/overloaded.hpp"
#include "dsl/semantic/detail/traversal.hpp"
#include "dsl/semantic/detail/types/type_rules.hpp"

namespace dsl::semantic::detail {

void Traversal::visit_globals(const std::vector<ast::GlobalItem>& globals) {
    collect_global_patterns(globals);

    for (const auto& item : globals) {
        std::visit(utils::overloaded{
                       [&](const ast::StatementPtr& statement) {
                           if (statement) {
                               visit_statement(*statement);
                           }
                       },
                       [&](const ast::PatternDefinition& pattern) { visit_pattern(pattern); },
                   },
                   item);
    }
}

void Traversal::visit_track(const ast::TrackDefinition& track) {
    if (track.name) {
        (void)scopes_.add_symbol(*track.name, SymbolKind::Track, Type{TypeKind::Void}, track.location, &track);
    }

    ScopeStack::Guard guard(scopes_);

    collect_track_patterns(track.body);
    for (const auto& item : track.body) {
        std::visit(utils::overloaded{
                       [&](const ast::StatementPtr& statement) {
                           if (statement) {
                               visit_statement(*statement);
                           }
                       },
                       [&](const ast::PatternDefinition& pattern) { visit_pattern(pattern); },
                       [&](const ast::VoiceDefinition& voice) { visit_voice(voice); },
                   },
                   item);
    }
}

void Traversal::visit_voice(const ast::VoiceDefinition& voice) {
    if (voice.from_expression) {
        if (const Type from_type = visit_expression(**voice.from_expression);
            is_known(from_type) && !is_numeric(from_type)) {
            diagnose(voice.location, "voice 'from' expression must be numeric");
        }
    }

    ScopeStack::Guard guard(scopes_);

    collect_voice_patterns(voice.body);
    for (const auto& item : voice.body) {
        std::visit(utils::overloaded{
                       [&](const ast::StatementPtr& statement) {
                           if (statement) {
                               visit_statement(*statement);
                           }
                       },
                       [&](const ast::PatternDefinition& pattern) { visit_pattern(pattern); },
                   },
                   item);
    }
}

void Traversal::visit_pattern(const ast::PatternDefinition& pattern) {
    ScopeStack::Guard guard(scopes_);

    for (const auto& param : pattern.params) {
        (void)scopes_.add_symbol(param, SymbolKind::Parameter, Type{TypeKind::Unknown}, pattern.location, &pattern);
    }

    visit_block(pattern.body);
}

}  // namespace dsl::semantic::detail
