#include "dsl/semantic/detail/traversal.hpp"

namespace dsl::semantic::detail {

void Traversal::collect_global_patterns(const std::vector<ast::GlobalItem>& globals) const {
    for (const auto& item : globals) {
        if (const auto* pattern = std::get_if<ast::PatternDefinition>(&item)) {
            add_pattern_symbol(*pattern);
        }
    }
}

void Traversal::collect_track_patterns(const std::vector<ast::TrackItem>& items) const {
    for (const auto& item : items) {
        if (const auto* pattern = std::get_if<ast::PatternDefinition>(&item)) {
            add_pattern_symbol(*pattern);
        }
    }
}

void Traversal::collect_voice_patterns(const std::vector<ast::VoiceItem>& items) const {
    for (const auto& item : items) {
        if (const auto* pattern = std::get_if<ast::PatternDefinition>(&item)) {
            add_pattern_symbol(*pattern);
        }
    }
}

}  // namespace dsl::semantic::detail
