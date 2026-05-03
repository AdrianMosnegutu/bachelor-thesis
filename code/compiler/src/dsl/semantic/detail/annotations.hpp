#pragma once

#include <optional>
#include <unordered_map>

#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ast/statements.hpp"
#include "dsl/semantic/detail/expression_annotation.hpp"
#include "dsl/semantic/symbol.hpp"
#include "dsl/semantic/type.hpp"

namespace dsl::semantic::detail {

class Annotations {
   public:
    void set_expression_type(const ast::Expression& expression, Type type);
    void set_resolved_symbol(const ast::Expression& expression, SymbolId symbol);
    void set_assign_target(const ast::AssignStatement& assign, SymbolId symbol);

    [[nodiscard]] std::optional<Type> get_expression_type(const ast::Expression& expression) const;
    [[nodiscard]] std::optional<SymbolId> get_resolved_symbol(const ast::Expression& expression) const;
    [[nodiscard]] std::optional<SymbolId> get_assign_target(const ast::AssignStatement& assign) const;

    [[nodiscard]] bool is_empty() const;
    [[nodiscard]] std::size_t expression_type_count() const;
    [[nodiscard]] std::size_t resolved_symbol_count() const;

   private:
    using AnnotationMap = std::unordered_map<const ast::Expression*, detail::ExpressionAnnotation>;
    using AssignTargetMap = std::unordered_map<const ast::AssignStatement*, SymbolId>;

    AnnotationMap annotations_;
    AssignTargetMap assign_targets_;
};

}  // namespace dsl::semantic::detail
