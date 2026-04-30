#include "dsl/semantic/detail/scopes/scope_stack.hpp"

#include <cassert>
#include <utility>

namespace dsl::semantic::detail {

ScopeStack::ScopeStack(SymbolTable& symbols) : symbols_(symbols) {}

ScopeId ScopeStack::push_scope() {
    const auto parent = stack_.empty() ? std::optional<ScopeId>{} : std::optional{stack_.back()};
    const ScopeId scope = symbols_.add_scope(parent);
    stack_.push_back(scope);
    return scope;
}

void ScopeStack::pop_scope() {
    assert(!stack_.empty());
    stack_.pop_back();
}

ScopeId ScopeStack::current_scope() const {
    assert(!stack_.empty());
    return stack_.back();
}

const Symbol* ScopeStack::find_visible(const std::string& name) const {
    return symbols_.find_visible(current_scope(), name);
}

const Symbol* ScopeStack::find_visible(const std::string& name, const std::initializer_list<SymbolKind> kinds) const {
    return symbols_.find_visible(current_scope(), name, kinds);
}

SymbolId ScopeStack::add_symbol(const std::string& name,
                                const SymbolKind kind,
                                const Type type,
                                const Location& location,
                                const void* declaration) const {
    return symbols_.add_symbol(current_scope(), name, kind, type, location, declaration);
}

}  // namespace dsl::semantic::detail
