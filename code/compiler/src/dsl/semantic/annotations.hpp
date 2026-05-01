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
    void set_resolved_symbol(const ast::Expression& expression, SymbolId symbol);

    [[nodiscard]] std::optional<Type> get_expression_type(const ast::Expression& expression) const;
    [[nodiscard]] std::optional<SymbolId> get_resolved_symbol(const ast::Expression& expression) const;

    [[nodiscard]] bool is_empty() const;
    [[nodiscard]] std::size_t expression_type_count() const;
    [[nodiscard]] std::size_t resolved_symbol_count() const;

   private:
    using TypeMap = std::unordered_map<const ast::Expression*, Type>;
    using SymbolMap = std::unordered_map<const ast::Expression*, SymbolId>;

    TypeMap expression_types_;
    SymbolMap resolved_symbols_;
};

}  // namespace dsl::semantic
