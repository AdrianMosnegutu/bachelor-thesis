#include "dsl/ir/expression_evaluator.hpp"

#include <type_traits>
#include <variant>

#include "dsl/ast/expr.hpp"
#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/value.hpp"
#include "dsl/utils/overloaded.hpp"

namespace dsl::ir {

namespace {

template <typename T>
concept Literal =
    std::is_same_v<T, ast::IntLiteral> || std::is_same_v<T, ast::FloatLiteral> || std::is_same_v<T, ast::BoolLiteral> ||
    std::is_same_v<T, ast::RestLiteral> || std::is_same_v<T, ast::NoteLiteral>;

}  // namespace

Value evaluate_expression(const ast::Expression& expression, LowererContext& context) {
    return std::visit(
        utils::overloaded{
            [&](const Literal auto& literal) -> Value { return detail::evaluate_literal(literal); },
            [&](const ast::UnaryExpression& expr) -> Value {
                return detail::evaluate_unary(expr, expression.loc, context);
            },
            [&](const ast::BinaryExpression& expr) -> Value {
                return detail::evaluate_binary(expr, expression.loc, context);
            },
            [&](const ast::TernaryExpression& expr) -> Value {
                return detail::evaluate_ternary(expr, expression.loc, context);
            },
            [&](const ast::Identifier& identifier) -> Value {
                return detail::evaluate_identifier(identifier, expression.loc, context);
            },
            [&](const ast::Sequence& sequence) -> Value { return detail::evaluate_sequence(sequence, context); },
            [&](const ast::Chord& chord) -> Value { return detail::evaluate_chord(chord, expression.loc, context); },
            [&](const ast::Call& call) -> Value { return detail::evaluate_call(call, expression.loc, context); },
            [&](const ast::ParenthesisedExpression& paran) -> Value {
                return evaluate_expression(*paran.inner, context);
            },
        },
        expression.kind);
}

}  // namespace dsl::ir
