#include "dsl/ir/lowerer_context.hpp"

#include <cassert>
#include <ranges>

#include "dsl/errors/semantic_error.hpp"

namespace dsl::ir {

using errors::SemanticError;

void LowererContext::push_scope() { scope_stack_.emplace_back(); }

void LowererContext::pop_scope() {
    assert(!scope_stack_.empty());
    scope_stack_.pop_back();
}

void LowererContext::bind(const std::string& name, Value val) {
    assert(!scope_stack_.empty());
    scope_stack_.back()[name] = std::move(val);
}

const Value& LowererContext::lookup(const std::string& name, const Location& loc) const {
    for (const auto& it : std::views::reverse(scope_stack_)) {
        if (auto found = it.find(name); found != it.end()) {
            return found->second;
        }
    }
    throw SemanticError(loc, "undefined variable '" + name + "'");
}

void LowererContext::collect_patterns(const std::vector<ast::GlobalItem>& globals) {
    for (const auto& item : globals) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            patterns_[pat->name] = pat;
        }
    }
}

void LowererContext::collect_track_patterns(const std::vector<ast::TrackItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            patterns_[pat->name] = pat;
        }
    }
}

void LowererContext::erase_track_patterns(const std::vector<ast::TrackItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            patterns_.erase(pat->name);
        }
    }
}

void LowererContext::collect_voice_patterns(const std::vector<ast::VoiceItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            patterns_[pat->name] = pat;
        }
    }
}

void LowererContext::erase_voice_patterns(const std::vector<ast::VoiceItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            patterns_.erase(pat->name);
        }
    }
}

void LowererContext::assign(const std::string& name, Value val, const Location& loc) {
    for (auto& it : std::ranges::views::reverse(scope_stack_)) {
        if (auto found = it.find(name); found != it.end()) {
            found->second = std::move(val);
            return;
        }
    }
    throw SemanticError(loc, "assignment to undeclared variable '" + name + "'");
}

const ast::PatternDefinition* LowererContext::find_pattern(const std::string& name) const {
    const auto it = patterns_.find(name);
    return it != patterns_.end() ? it->second : nullptr;
}

}  // namespace dsl::ir
