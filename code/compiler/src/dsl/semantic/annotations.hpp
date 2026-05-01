#pragma once

#include <optional>
#include <unordered_map>

#include "dsl/ast/expressions.hpp"
#include "dsl/semantic/symbol.hpp"
#include "dsl/semantic/type.hpp"

namespace dsl::semantic {

class Annotations {
   public:
    void set_expression_type(const ast::Expression& expression, Type type);
    [[nodiscard]] std::optional<Type> expression_type(const ast::Expression& expression) const;

    void set_resolved_symbol(const ast::Expression& expression, SymbolId symbol);
    [[nodiscard]] std::optional<SymbolId> resolved_symbol(const ast::Expression& expression) const;

    [[nodiscard]] bool empty() const { return expression_types_.empty() && resolved_symbols_.empty(); }
    [[nodiscard]] std::size_t expression_type_count() const { return expression_types_.size(); }
    [[nodiscard]] std::size_t resolved_symbol_count() const { return resolved_symbols_.size(); }

   private:
    std::unordered_map<const ast::Expression*, Type> expression_types_;
    std::unordered_map<const ast::Expression*, SymbolId> resolved_symbols_;
};

}  // namespace dsl::semantic
