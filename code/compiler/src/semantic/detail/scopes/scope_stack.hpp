#pragma once

#include <initializer_list>
#include <vector>

#include "dsl/semantic/symbol_table.hpp"

namespace dsl::semantic::detail {

class ScopeStack {
   public:
    explicit ScopeStack(SymbolTable& symbols);

    ScopeId push_scope();
    void pop_scope();

    [[nodiscard]] ScopeId current_scope() const;
    [[nodiscard]] const Symbol* find_visible(const std::string& name) const;
    [[nodiscard]] const Symbol* find_visible(const std::string& name,
                                             std::initializer_list<SymbolKind> kinds) const;

    [[nodiscard]] SymbolId add_symbol(const std::string& name,
                                      SymbolKind kind,
                                      Type type,
                                      Location location,
                                      const void* declaration = nullptr);

   private:
    SymbolTable& symbols_;
    std::vector<ScopeId> stack_;
};

}  // namespace dsl::semantic::detail
