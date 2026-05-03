#pragma once

#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"

namespace dsl::lowering::detail {

ir::Value evaluate_literal_expression(const ast::IntLiteralExpression& literal);
ir::Value evaluate_literal_expression(const ast::FloatLiteralExpression& literal);
ir::Value evaluate_literal_expression(const ast::BoolLiteralExpression& literal);
ir::Value evaluate_literal_expression(const ast::RestLiteralExpression& literal);
ir::Value evaluate_literal_expression(const ast::NoteLiteralExpression& literal);

ir::Value evaluate_unary_expression(const ast::UnaryExpression& unary,
                                    const source::Location& loc,
                                    LowererContext& context);
ir::Value evaluate_binary_expression(const ast::BinaryExpression& binary,
                                     const source::Location& loc,
                                     LowererContext& context);
ir::Value evaluate_ternary_expression(const ast::TernaryExpression& ternary, LowererContext& context);

ir::Value evaluate_identifier_expression(const ast::Expression& expression,
                                         const source::Location& loc,
                                         const LowererContext& context);

ir::Value evaluate_chord_expression(const ast::ChordExpression& chord,
                                    const source::Location& loc,
                                    LowererContext& context);
ir::Value evaluate_sequence_expression(const ast::SequenceExpression& sequence, LowererContext& context);
ir::Value evaluate_pattern_call_expression(const ast::Expression& expression,
                                           const ast::PatternCallExpression& call,
                                           const source::Location& loc,
                                           LowererContext& context);

[[nodiscard]] ir::Value evaluate_expression(const ast::Expression& expression, LowererContext& context);

}  // namespace dsl::lowering::detail
