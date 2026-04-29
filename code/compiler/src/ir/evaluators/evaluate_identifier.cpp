#include "dsl/ir/expression_evaluator.hpp"

namespace dsl::ir::detail {

Value evaluate_identifier(const ast::IdentifierExpression& identifier, const Location& loc, const LowererContext& context) {
    return context.lookup(identifier.name, loc);
}

}  // namespace dsl::ir::detail
