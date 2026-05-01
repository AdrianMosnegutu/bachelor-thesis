#pragma once

#include <initializer_list>
#include <stack>

#include "dsl/semantic/symbol_table.hpp"

namespace dsl::semantic::detail {

class ScopeStack {
   public:
    class Guard {
       public:
        Guard(ScopeStack& scope_stack);
        ~Guard();

        [[nodiscard]] ScopeId get_scope_id() const;

       private:
        ScopeStack& scope_stack_;
        ScopeId scope_id_;
    };

    explicit ScopeStack(SymbolTable& symbols);

    ScopeId push_scope();
    void pop_scope();

    [[nodiscard]] ScopeId current_scope() const;
    [[nodiscard]] const Symbol* find_visible(const std::string& name) const;
    [[nodiscard]] const Symbol* find_visible(const std::string& name, std::initializer_list<SymbolKind> kinds) const;

    SymbolId add_symbol(const std::string& name,
                        SymbolKind kind,
                        Type type,
                        const source::Location& location,
                        const void* declaration = nullptr) const;

   private:
    SymbolTable& symbols_;
    std::stack<ScopeId> stack_;
};

}  // namespace dsl::semantic::detail
