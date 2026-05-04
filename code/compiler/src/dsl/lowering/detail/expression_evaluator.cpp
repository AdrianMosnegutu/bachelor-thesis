#include "dsl/lowering/detail/expression_evaluator.hpp"

#include <variant>

#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/common/utils/overloaded.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"

namespace dsl::lowering::detail {

namespace {

template <typename T>
concept Literal = std::same_as<T, ast::IntLiteralExpression> || std::same_as<T, ast::FloatLiteralExpression> ||
                  std::same_as<T, ast::BoolLiteralExpression> || std::same_as<T, ast::NoteLiteralExpression> ||
                  std::same_as<T, ast::RestLiteralExpression> || std::same_as<T, ast::DrumNoteLiteralExpression>;

}

ir::Value evaluate_expression(const ast::Expression& expression, LowererContext& context) {
    return std::visit(
        utils::overloaded{
            [&](const Literal auto& kind) -> ir::Value { return evaluate_literal_expression(kind); },
            [&](const ast::UnaryExpression& kind) -> ir::Value {
                return evaluate_unary_expression(kind, expression.location, context);
            },
            [&](const ast::BinaryExpression& kind) -> ir::Value {
                return evaluate_binary_expression(kind, expression.location, context);
            },
            [&](const ast::TernaryExpression& kind) -> ir::Value { return evaluate_ternary_expression(kind, context); },
            [&](const ast::IdentifierExpression& kind) -> ir::Value {
                return evaluate_identifier_expression(expression, expression.location, context);
            },
            [&](const ast::SequenceExpression& kind) -> ir::Value {
                return evaluate_sequence_expression(kind, context);
            },
            [&](const ast::ChordExpression& kind) -> ir::Value {
                return evaluate_chord_expression(kind, expression.location, context);
            },
            [&](const ast::PatternCallExpression& kind) -> ir::Value {
                return evaluate_pattern_call_expression(expression, kind, expression.location, context);
            },
            [&](const ast::ParenthesisedExpression& kind) -> ir::Value {
                return evaluate_expression(*kind.inner, context);
            },
        },
        expression.kind);
}

}  // namespace dsl::lowering::detail
