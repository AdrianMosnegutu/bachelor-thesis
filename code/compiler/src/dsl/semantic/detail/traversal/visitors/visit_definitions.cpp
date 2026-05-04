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

    const auto prev_instrument = current_track_instrument_;
    current_track_instrument_ = track.instrument;
    const ScopeId prev_boundary = writable_boundary_;
    writable_boundary_ = scopes_.current_scope();

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

    current_track_instrument_ = prev_instrument;
    writable_boundary_ = prev_boundary;
}

void Traversal::visit_voice(const ast::VoiceDefinition& voice) {
    if (voice.from_expression) {
        if (const Type from_type = visit_expression(**voice.from_expression);
            is_known(from_type) && !is_numeric(from_type)) {
            diagnose(voice.location, "voice 'from' expression must be numeric");
        }
    }

    ScopeStack::Guard guard(scopes_);

    const ScopeId prev_boundary = writable_boundary_;
    writable_boundary_ = scopes_.current_scope();

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

    writable_boundary_ = prev_boundary;
}

void Traversal::visit_pattern(const ast::PatternDefinition& pattern) {
    ScopeStack::Guard guard(scopes_);

    // Only set a new boundary if there is none yet (global pattern context).
    // Patterns inside a track/voice inherit the enclosing boundary so they can
    // write to the same scope level as the surrounding body.
    const ScopeId prev_boundary = writable_boundary_;
    const bool set_boundary = (writable_boundary_ == INVALID_SCOPE_ID);
    if (set_boundary) {
        writable_boundary_ = scopes_.current_scope();
    }

    for (const auto& param : pattern.params) {
        (void)scopes_.add_symbol(param, SymbolKind::Parameter, Type{TypeKind::Unknown}, pattern.location, &param);
    }

    visit_block(pattern.body);

    writable_boundary_ = prev_boundary;
}

}  // namespace dsl::semantic::detail
