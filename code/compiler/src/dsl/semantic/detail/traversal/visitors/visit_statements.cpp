#include <variant>

#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/utils/overloaded.hpp"
#include "dsl/semantic/detail/traversal.hpp"
#include "dsl/semantic/detail/types/type_rules.hpp"

namespace dsl::semantic::detail {

void Traversal::visit_block(const ast::Block& block) {
    for (const auto& statement : block) {
        if (statement) {
            visit_statement(*statement);
        }
    }
}

void Traversal::visit_statement(const ast::Statement& statement) {
    std::visit(utils::overloaded{
                   [&](const ast::AssignStatement& assign) { visit_assign_statement(assign, statement.location); },
                   [&](const ast::ForStatement& for_stmt) { visit_for_statement(for_stmt, statement.location); },
                   [&](const ast::IfStatement& if_stmt) { visit_if_statement(if_stmt, statement.location); },
                   [&](const ast::LetStatement& let) { visit_let_statement(let, statement.location); },
                   [&](const ast::LoopStatement& loop) { visit_loop_statement(loop, statement.location); },
                   [&](const ast::PlayStatement& play) { visit_play_target(play.target); },
               },
               statement.kind);
}

void Traversal::visit_assign_statement(const ast::AssignStatement& assign, const source::Location& location) {
    const auto* symbol = scopes_.find_visible(assign.name, {SymbolKind::Variable, SymbolKind::Parameter});
    const SymbolId symbol_id = symbol ? symbol->id : INVALID_SYMBOL_ID;

    if (!symbol) {
        diagnose(location, "assignment to undeclared variable '" + assign.name + "'");
    }

    const Type value_type = visit_expression(*assign.value);
    if (symbol_id != INVALID_SYMBOL_ID) {
        result_.symbols().set_symbol_type(symbol_id, value_type);
    }
}

void Traversal::visit_for_statement(const ast::ForStatement& for_stmt, const source::Location& location) {
    ScopeStack::Guard guard(scopes_);

    if (for_stmt.init) {
        visit_statement(*for_stmt.init);
    }

    if (for_stmt.condition) {
        const Type condition_type = visit_expression(*for_stmt.condition);

        if (is_known(condition_type) && !is_boolean(condition_type)) {
            diagnose(for_stmt.condition->location, "for condition must be a boolean");
        }
    }

    visit_block(for_stmt.body);
    if (for_stmt.step) {
        visit_statement(*for_stmt.step);
    }
}

void Traversal::visit_loop_statement(const ast::LoopStatement& loop, const source::Location& location) {
    const Type count_type = visit_expression(*loop.count);

    if (is_known(count_type) && !is_integral(count_type)) {
        diagnose(loop.count->location, "loop count must be an integer");
    }

    visit_block(loop.body);
}

void Traversal::visit_if_statement(const ast::IfStatement& if_stmt, const source::Location& location) {
    const Type condition_type = visit_expression(*if_stmt.condition);

    if (is_known(condition_type) && !is_boolean(condition_type)) {
        diagnose(if_stmt.condition->location, "if condition must be a boolean");
    }

    visit_block(if_stmt.then_branch);
    if (if_stmt.else_branch) {
        visit_block(*if_stmt.else_branch);
    }
}

void Traversal::visit_let_statement(const ast::LetStatement& let, const source::Location& location) {
    const Type value_type = visit_expression(*let.value);
    (void)scopes_.add_symbol(let.name, SymbolKind::Variable, value_type, location, &let);
}

void Traversal::visit_play_target(const ast::PlayTarget& target) {
    std::visit(utils::overloaded{
                   [&](const ast::ExpressionPtr& expression) {
                       if (expression) {
                           visit_expression(*expression);
                       }
                   },
                   [](const auto&) {},
               },
               target.source);

    if (target.duration) {
        const Type duration_type = visit_expression(*target.duration);

        if (is_known(duration_type) && !is_numeric(duration_type)) {
            diagnose(target.duration->location, "play duration must be a number");
        }
    }

    if (target.from_offset) {
        const Type from_type = visit_expression(*target.from_offset);

        if (is_known(from_type) && !is_numeric(from_type)) {
            diagnose(target.from_offset->location, "from offset must be a number");
        }
    }
}

}  // namespace dsl::semantic::detail
