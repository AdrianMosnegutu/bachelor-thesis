#include "dsl/ir/expression_evaluator.hpp"

#include <variant>

#include "dsl/core/ast/expression.hpp"
#include "dsl/core/utils/overloaded.hpp"
#include "dsl/ir/lowerer_context.hpp"
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
                          [&](const Literal auto& lit) -> Value { return detail::evaluate_literal(lit); },
                          [&](const ast::UnaryExpression& expr) -> Value {
                              return detail::evaluate_unary(expr, expression.location, context);
                          },
                          [&](const ast::BinaryExpression& expr) -> Value {
                              return detail::evaluate_binary(expr, expression.location, context);
                          },
                          [&](const ast::TernaryExpression& expr) -> Value {
                              return detail::evaluate_ternary(expr, expression.location, context);
                          },
                          [&](const ast::IdentifierExpression& identifier) -> Value {
                              return detail::evaluate_identifier(identifier, expression.location, context);
                          },
                          [&](const ast::SequenceExpression& sequence) -> Value {
                              return detail::evaluate_sequence(sequence, context);
                          },
                          [&](const ast::ChordExpression& chord) -> Value {
                              return detail::evaluate_chord(chord, expression.location, context);
                          },
                          [&](const ast::CallExpression& call) -> Value {
                              return detail::evaluate_call(call, expression.location, context);
                          },
                          [&](const ast::ParenthesisedExpression& paran) -> Value {
                              return evaluate_expression(*paran.inner, context);
                          },
                      },
                      expression.kind);
}

}  // namespace dsl::ir
