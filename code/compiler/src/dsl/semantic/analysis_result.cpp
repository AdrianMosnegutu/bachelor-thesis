#include "dsl/semantic/analysis_result.hpp"

#include <algorithm>
#include <memory>

#include "dsl/common/ast/program.hpp"
#include "dsl/semantic/detail/annotations.hpp"
#include "dsl/semantic/detail/symbol_table.hpp"

namespace dsl::semantic {

AnalysisResult::AnalysisResult(const ast::Program& program)
    : program_(&program),
      symbols_(std::make_unique<detail::SymbolTable>()),
      annotations_(std::make_unique<detail::Annotations>()) {}

AnalysisResult::AnalysisResult(AnalysisResult&&) noexcept = default;
AnalysisResult& AnalysisResult::operator=(AnalysisResult&&) noexcept = default;
AnalysisResult::~AnalysisResult() = default;

const ast::Program& AnalysisResult::program() const { return *program_; }

std::optional<Type> AnalysisResult::get_expression_type(const ast::Expression& expression) const {
    return annotations_->get_expression_type(expression);
}

std::optional<SymbolId> AnalysisResult::get_resolved_symbol(const ast::Expression& expression) const {
    return annotations_->get_resolved_symbol(expression);
}

const Symbol* AnalysisResult::get_symbol_by_id(const SymbolId id) const { return symbols_->get_symbol(id); }

const Symbol* AnalysisResult::get_symbol_by_declaration(const void* declaration) const {
    const auto& syms = symbols_->symbols();
    const auto it = std::ranges::find_if(syms, [declaration](const Symbol& s) { return s.declaration == declaration; });
    return it != syms.end() ? &*it : nullptr;
}

SymbolId AnalysisResult::get_assign_target(const ast::AssignStatement& assign) const {
    return annotations_->get_assign_target(assign).value_or(INVALID_SYMBOL_ID);
}

}  // namespace dsl::semantic
