#include "dsl/semantic/detail/annotations.hpp"

#include <algorithm>

namespace dsl::semantic::detail {

void Annotations::set_expression_type(const ast::Expression& expression, const Type type) {
    annotations_[&expression].type = type;
}

void Annotations::set_resolved_symbol(const ast::Expression& expression, const SymbolId symbol) {
    annotations_[&expression].resolved_symbol = symbol;
}

std::optional<Type> Annotations::get_expression_type(const ast::Expression& expression) const {
    if (const auto it = annotations_.find(&expression); it != annotations_.end()) {
        return it->second.type;
    }

    return std::nullopt;
}

std::optional<SymbolId> Annotations::get_resolved_symbol(const ast::Expression& expression) const {
    if (const auto it = annotations_.find(&expression); it != annotations_.end()) {
        return it->second.resolved_symbol;
    }

    return std::nullopt;
}

void Annotations::set_assign_target(const ast::AssignStatement& assign, const SymbolId symbol) {
    assign_targets_[&assign] = symbol;
}

std::optional<SymbolId> Annotations::get_assign_target(const ast::AssignStatement& assign) const {
    if (const auto it = assign_targets_.find(&assign); it != assign_targets_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Annotations::is_empty() const { return annotations_.empty(); }

std::size_t Annotations::expression_type_count() const { return annotations_.size(); }

std::size_t Annotations::resolved_symbol_count() const {
    return std::ranges::count_if(annotations_,
                                 [](const auto& entry) { return entry.second.resolved_symbol.has_value(); });
}

}  // namespace dsl::semantic::detail
