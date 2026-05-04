#include "dsl/semantic/detail/scopes/scope_stack.hpp"

#include <cassert>

namespace dsl::semantic::detail {

ScopeStack::ScopeStack(SymbolTable& symbols) : symbols_(symbols) {}

ScopeId ScopeStack::push_scope() {
    const auto parent = stack_.empty() ? std::optional<ScopeId>{} : std::optional{stack_.top()};
    const ScopeId scope = symbols_.add_scope(parent);

    stack_.push(scope);
    return scope;
}

void ScopeStack::pop_scope() {
    assert(!stack_.empty());
    stack_.pop();
}

ScopeId ScopeStack::current_scope() const {
    assert(!stack_.empty());
    return stack_.top();
}

const Symbol* ScopeStack::find_in_current_scope(const std::string& name) const {
    return symbols_.find_in_scope(current_scope(), name);
}

const Symbol* ScopeStack::find_in_current_scope_by_arity(const std::string& name, const std::size_t arity) const {
    return symbols_.find_in_scope_by_arity(current_scope(), name, arity);
}

const Symbol* ScopeStack::find_pattern_visible_by_arity(const std::string& name, const std::size_t arity) const {
    return symbols_.find_visible_by_arity(current_scope(), name, arity);
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
                                const source::Location& location,
                                const void* declaration) const {
    return symbols_.add_symbol(current_scope(), name, kind, type, location, declaration);
}

ScopeStack::Guard::Guard(ScopeStack& scope_stack) : scope_stack_(scope_stack) { scope_stack.push_scope(); }

ScopeStack::Guard::~Guard() { scope_stack_.pop_scope(); }

[[nodiscard]] ScopeId ScopeStack::Guard::get_scope_id() const { return scope_id_; }

}  // namespace dsl::semantic::detail
