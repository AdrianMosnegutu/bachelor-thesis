#include "dsl/common/ast/expressions.hpp"
#include "dsl/common/ir/values.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"

namespace dsl::lowering::detail {

ir::Value evaluate_identifier_expression(const ast::Expression& expression,
                                         const source::Location& loc,
                                         const LowererContext& context) {
    const auto symbol_id = context.analysis().get_resolved_symbol(expression);
    if (!symbol_id) {
        throw LoweringFailure(loc, "lowering reached identifier with no resolved symbol");
    }

    return context.lookup(*symbol_id, loc);
}

}  // namespace dsl::lowering::detail
