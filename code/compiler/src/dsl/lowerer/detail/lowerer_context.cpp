#include "dsl/lowerer/detail/lowerer_context.hpp"

#include <cassert>
#include <ranges>
#include <sstream>
#include <utility>

#include "dsl/semantic/symbol.hpp"

namespace dsl::lowerer::detail {

namespace {

std::string format_lowerer_error(const source::Location& loc, const std::string& msg) {
    std::ostringstream stream;
    stream << loc << ": " << msg;
    return stream.str();
}

}  // namespace

LoweringFailure::LoweringFailure(const source::Location& loc, const std::string& msg)
    : std::runtime_error(format_lowerer_error(loc, msg)) {}

LowererContext::LowererContext(const semantic::AnalysisResult& analysis, DiagnosticsEngine& diagnostics)
    : analysis_(analysis), diagnostics_(diagnostics) {}

const semantic::AnalysisResult& LowererContext::analysis() const { return analysis_; }

void LowererContext::push_scope() { scope_stack_.emplace_back(); }

void LowererContext::pop_scope() {
    assert(!scope_stack_.empty());
    scope_stack_.pop_back();
}

void LowererContext::bind(const semantic::SymbolId id, ir::Value val) {
    assert(!scope_stack_.empty());
    scope_stack_.back()[id] = std::move(val);
}

const ir::Value& LowererContext::lookup(const semantic::SymbolId id, const source::Location& loc) const {
    for (const auto& scope : std::views::reverse(scope_stack_)) {
        if (auto found = scope.find(id); found != scope.end()) {
            return found->second;
        }
    }
    throw LoweringFailure(loc, "lowering reached unresolved variable (id=" + std::to_string(id) + ")");
}

void LowererContext::assign(const semantic::SymbolId id, ir::Value val, const source::Location& loc) {
    for (auto& scope : std::views::reverse(scope_stack_)) {
        if (auto found = scope.find(id); found != scope.end()) {
            found->second = std::move(val);
            return;
        }
    }
    throw LoweringFailure(loc, "lowering reached assignment to unresolved variable (id=" + std::to_string(id) + ")");
}

void LowererContext::collect_patterns(const std::vector<ast::GlobalItem>& globals) {
    for (const auto& item : globals) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            if (const auto* sym = analysis_.get_symbol_by_declaration(pat)) {
                patterns_[sym->id] = pat;
            }
        }
    }
}

void LowererContext::collect_track_patterns(const std::vector<ast::TrackItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            if (const auto* sym = analysis_.get_symbol_by_declaration(pat)) {
                patterns_[sym->id] = pat;
            }
        }
    }
}

void LowererContext::erase_track_patterns(const std::vector<ast::TrackItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            if (const auto* sym = analysis_.get_symbol_by_declaration(pat)) {
                patterns_.erase(sym->id);
            }
        }
    }
}

void LowererContext::collect_voice_patterns(const std::vector<ast::VoiceItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            if (const auto* sym = analysis_.get_symbol_by_declaration(pat)) {
                patterns_[sym->id] = pat;
            }
        }
    }
}

void LowererContext::erase_voice_patterns(const std::vector<ast::VoiceItem>& items) {
    for (const auto& item : items) {
        if (const auto* pat = std::get_if<ast::PatternDefinition>(&item)) {
            if (const auto* sym = analysis_.get_symbol_by_declaration(pat)) {
                patterns_.erase(sym->id);
            }
        }
    }
}

const ast::PatternDefinition* LowererContext::find_pattern(const semantic::SymbolId id) const {
    const auto it = patterns_.find(id);
    return it != patterns_.end() ? it->second : nullptr;
}

void LowererContext::report_lowering_error(std::string message) {
    diagnostics_.report(DiagnosticStage::Lowering, DiagnosticSeverity::Error, std::move(message));
}

LowererScopeGuard::LowererScopeGuard(LowererContext& ctx) : ctx_(ctx) { ctx_.push_scope(); }

LowererScopeGuard::~LowererScopeGuard() { ctx_.pop_scope(); }

}  // namespace dsl::lowerer::detail
