#include "dsl/ir/lowerer/expression_evaluator.hpp"

#include <variant>

#include "dsl/core/ast/expression.hpp"
#include "dsl/core/utils/overloaded.hpp"
#include "dsl/ir/lowerer/lowerer_context.hpp"
#include "dsl/ir/value.hpp"

namespace dsl::ir {

namespace {

template <typename T>
concept Literal = std::same_as<T, ast::IntLiteralExpression> || std::same_as<T, ast::FloatLiteralExpression> ||
                  std::same_as<T, ast::BoolLiteralExpression> || std::same_as<T, ast::NoteLiteralExpression> ||
                  std::same_as<T, ast::RestLiteralExpression>;

}

Value evaluate_expression(const ast::Expression& expression, LowererContext& context) {
    return std::visit(utils::overloaded{
                          [&](const Literal auto& kind) -> Value { return detail::evaluate_literal_expression(kind); },
                          [&](const ast::UnaryExpression& kind) -> Value {
                              return detail::evaluate_unary_expression(kind, expression.location, context);
                          },
                          [&](const ast::BinaryExpression& kind) -> Value {
                              return detail::evaluate_binary_expression(kind, expression.location, context);
                          },
                          [&](const ast::TernaryExpression& kind) -> Value {
                              return detail::evaluate_ternary_expression(kind, expression.location, context);
                          },
                          [&](const ast::IdentifierExpression& kind) -> Value {
                              return detail::evaluate_identifier_expression(kind, expression.location, context);
                          },
                          [&](const ast::SequenceExpression& kind) -> Value {
                              return detail::evaluate_sequence_expression(kind, context);
                          },
                          [&](const ast::ChordExpression& kind) -> Value {
                              return detail::evaluate_chord_expression(kind, expression.location, context);
                          },
                          [&](const ast::CallExpression& kind) -> Value {
                              return detail::evaluate_call_expression(kind, expression.location, context);
                          },
                          [&](const ast::ParenthesisedExpression& kind) -> Value {
                              return evaluate_expression(*kind.inner, context);
                          },
                      },
                      expression.kind);
}

}  // namespace dsl::ir
