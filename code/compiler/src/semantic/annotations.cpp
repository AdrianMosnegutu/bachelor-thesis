#include "dsl/semantic/annotations.hpp"

namespace dsl::semantic {

void Annotations::set_expression_type(const ast::Expression& expression, const Type type) {
    expression_types_[&expression] = type;
}

std::optional<Type> Annotations::expression_type(const ast::Expression& expression) const {
    if (const auto it = expression_types_.find(&expression); it != expression_types_.end()) {
        return it->second;
    }

    return std::nullopt;
}

void Annotations::set_resolved_symbol(const ast::Expression& expression, const SymbolId symbol) {
    resolved_symbols_[&expression] = symbol;
}

std::optional<SymbolId> Annotations::resolved_symbol(const ast::Expression& expression) const {
    if (const auto it = resolved_symbols_.find(&expression); it != resolved_symbols_.end()) {
        return it->second;
    }

    return std::nullopt;
}

}  // namespace dsl::semantic
