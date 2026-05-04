#pragma once

#include <initializer_list>
#include <stack>

#include "dsl/semantic/detail/symbol_table.hpp"

namespace dsl::semantic::detail {

class ScopeStack {
   public:
    class Guard {
       public:
        Guard(ScopeStack& scope_stack);
        ~Guard();

        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;

        [[nodiscard]] ScopeId get_scope_id() const;

       private:
        ScopeStack& scope_stack_;
        ScopeId scope_id_;
    };

    explicit ScopeStack(SymbolTable& symbols);

    ScopeId push_scope();
    void pop_scope();

    [[nodiscard]] ScopeId current_scope() const;
    [[nodiscard]] const Symbol* find_in_current_scope(const std::string& name) const;
    [[nodiscard]] const Symbol* find_in_current_scope_by_arity(const std::string& name, std::size_t arity) const;
    [[nodiscard]] const Symbol* find_visible(const std::string& name) const;
    [[nodiscard]] const Symbol* find_visible(const std::string& name, std::initializer_list<SymbolKind> kinds) const;
    [[nodiscard]] const Symbol* find_pattern_visible_by_arity(const std::string& name, std::size_t arity) const;

    SymbolId add_symbol(const std::string& name,
                        SymbolKind kind,
                        Type type,
                        const source::Location& location,
                        const void* declaration = nullptr) const;

    [[nodiscard]] const SymbolTable& symbol_table() const { return symbols_; }

   private:
    SymbolTable& symbols_;
    std::stack<ScopeId> stack_;
};

}  // namespace dsl::semantic::detail
