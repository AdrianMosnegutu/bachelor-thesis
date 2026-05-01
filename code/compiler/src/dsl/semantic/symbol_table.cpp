#include "dsl/semantic/symbol_table.hpp"

#include <cassert>
#include <ranges>
#include <utility>

namespace dsl::semantic {

ScopeId SymbolTable::add_scope(const std::optional<ScopeId> parent) {
    if (parent) {
        assert(*parent < scopes_.size());
    }

    const ScopeId id = scopes_.size();
    scopes_.emplace_back(id, parent);

    return id;
}

const Scope* SymbolTable::get_scope(const ScopeId id) const { return id < scopes_.size() ? &scopes_[id] : nullptr; }

SymbolId SymbolTable::add_symbol(const ScopeId scope_id,
                                 std::string name,
                                 const SymbolKind kind,
                                 const Type type,
                                 const source::Location& location,
                                 const void* declaration) {
    assert(scope_id < scopes_.size());

    const SymbolId id = symbols_.size();
    symbols_.emplace_back(id, name, kind, type, location, declaration);
    scopes_[scope_id].symbols[std::move(name)].push_back(id);

    return id;
}

const Symbol* SymbolTable::get_symbol(const SymbolId id) const {
    return id < symbols_.size() ? &symbols_[id] : nullptr;
}

Symbol* SymbolTable::get_symbol(const SymbolId id) { return id < symbols_.size() ? &symbols_[id] : nullptr; }

void SymbolTable::set_symbol_type(const SymbolId id, const Type type) {
    if (Symbol* target = get_symbol(id)) {
        target->type = type;
    }
}

const Symbol* SymbolTable::find_in_scope(const ScopeId scope_id, const std::string& name) const {
    const Scope* current_scope = get_scope(scope_id);
    if (!current_scope) {
        return nullptr;
    }

    const auto found = current_scope->symbols.find(name);
    if (found == current_scope->symbols.end() || found->second.empty()) {
        return nullptr;
    }

    return get_symbol(found->second.back());
}

const Symbol* SymbolTable::find_in_scope(const ScopeId scope_id,
                                         const std::string& name,
                                         const std::initializer_list<SymbolKind> kinds) const {
    const Scope* current_scope = get_scope(scope_id);
    if (!current_scope) {
        return nullptr;
    }

    const auto found = current_scope->symbols.find(name);
    if (found == current_scope->symbols.end()) {
        return nullptr;
    }

    for (const SymbolId id : std::views::reverse(found->second)) {
        const Symbol* candidate = get_symbol(id);
        if (!candidate) {
            continue;
        }

        for (const SymbolKind kind : kinds) {
            if (candidate->kind == kind) {
                return candidate;
            }
        }
    }

    return nullptr;
}

const Symbol* SymbolTable::find_visible(ScopeId scope_id, const std::string& name) const {
    while (const Scope* current_scope = get_scope(scope_id)) {
        if (const Symbol* found = find_in_scope(scope_id, name)) {
            return found;
        }

        if (!current_scope->parent) {
            break;
        }

        scope_id = *current_scope->parent;
    }

    return nullptr;
}

const Symbol* SymbolTable::find_visible(ScopeId scope_id,
                                        const std::string& name,
                                        const std::initializer_list<SymbolKind> kinds) const {
    while (const Scope* current_scope = get_scope(scope_id)) {
        if (const Symbol* found = find_in_scope(scope_id, name, kinds)) {
            return found;
        }

        if (!current_scope->parent) {
            break;
        }

        scope_id = *current_scope->parent;
    }

    return nullptr;
}

const std::vector<Scope>& SymbolTable::scopes() const { return scopes_; }

const std::vector<Symbol>& SymbolTable::symbols() const { return symbols_; }

}  // namespace dsl::semantic
