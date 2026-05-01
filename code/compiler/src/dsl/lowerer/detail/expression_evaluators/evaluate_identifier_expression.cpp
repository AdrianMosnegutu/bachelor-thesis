#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/lowerer/detail/lowerer_context.hpp"

namespace dsl::lowerer::detail {

ir::Value evaluate_identifier_expression(const ast::IdentifierExpression& identifier,
                                         const source::Location& loc,
                                         const LowererContext& context) {
    return context.lookup(identifier.name, loc);
}

}  // namespace dsl::lowerer::detail
