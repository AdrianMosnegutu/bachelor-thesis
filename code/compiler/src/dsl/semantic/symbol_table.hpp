#pragma once

#include <cstddef>
#include <initializer_list>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dsl/semantic/symbol.hpp"

namespace dsl::semantic {

using ScopeId = std::size_t;

inline constexpr ScopeId INVALID_SCOPE_ID = std::numeric_limits<ScopeId>::max();

struct Scope {
    ScopeId id = INVALID_SCOPE_ID;
    std::optional<ScopeId> parent;
    std::unordered_map<std::string, std::vector<SymbolId>> symbols;
};

class SymbolTable {
   public:
    [[nodiscard]] ScopeId add_scope(std::optional<ScopeId> parent = std::nullopt);
    [[nodiscard]] const Scope* get_scope(ScopeId id) const;

    [[nodiscard]] SymbolId add_symbol(ScopeId scope,
                                      std::string name,
                                      SymbolKind kind,
                                      Type type,
                                      const source::Location& location,
                                      const void* declaration = nullptr);
    [[nodiscard]] const Symbol* get_symbol(SymbolId id) const;
    [[nodiscard]] Symbol* get_symbol(SymbolId id);

    void set_symbol_type(SymbolId id, Type type);

    [[nodiscard]] const Symbol* find_in_scope(ScopeId scope, const std::string& name) const;
    [[nodiscard]] const Symbol* find_in_scope(ScopeId scope,
                                              const std::string& name,
                                              std::initializer_list<SymbolKind> kinds) const;

    [[nodiscard]] const Symbol* find_visible(ScopeId scope, const std::string& name) const;
    [[nodiscard]] const Symbol* find_visible(ScopeId scope,
                                             const std::string& name,
                                             std::initializer_list<SymbolKind> kinds) const;

    [[nodiscard]] const std::vector<Scope>& scopes() const;
    [[nodiscard]] const std::vector<Symbol>& symbols() const;

   private:
    std::vector<Scope> scopes_;
    std::vector<Symbol> symbols_;
};

}  // namespace dsl::semantic
