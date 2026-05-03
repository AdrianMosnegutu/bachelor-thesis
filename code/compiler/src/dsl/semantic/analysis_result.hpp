#pragma once

#include <memory>
#include <optional>

#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ast/program.hpp"
#include "dsl/semantic/symbol.hpp"
#include "dsl/semantic/type.hpp"

namespace dsl::semantic::detail {
class SymbolTable;
class Annotations;
class Traversal;
}  // namespace dsl::semantic::detail

namespace dsl::semantic {

class AnalysisResult {
    friend class detail::Traversal;

   public:
    explicit AnalysisResult(const ast::Program& program);

    AnalysisResult(AnalysisResult&&) noexcept;
    AnalysisResult(const AnalysisResult&) = delete;

    AnalysisResult& operator=(AnalysisResult&&) noexcept;
    AnalysisResult& operator=(const AnalysisResult&) = delete;

    ~AnalysisResult();

    [[nodiscard]] const ast::Program& program() const;

    [[nodiscard]] std::optional<Type> get_expression_type(const ast::Expression& expression) const;
    [[nodiscard]] std::optional<SymbolId> get_resolved_symbol(const ast::Expression& expression) const;
    [[nodiscard]] const Symbol* get_symbol_by_id(SymbolId id) const;
    [[nodiscard]] const Symbol* get_symbol_by_declaration(const void* declaration) const;
    [[nodiscard]] SymbolId get_assign_target(const ast::AssignStatement& assign) const;

   private:
    const ast::Program* program_;
    std::unique_ptr<detail::SymbolTable> symbols_;
    std::unique_ptr<detail::Annotations> annotations_;
};

}  // namespace dsl::semantic
