#include "dsl/ir/lowerer.hpp"

#include <algorithm>

#include "dsl/core/ast/statement.hpp"
#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/ir/detail/lowerer/ast_lowerers.h"
#include "dsl/ir/program.hpp"

namespace dsl::ir {

using errors::SemanticError;

Program lower(const ast::Program& program) {
    Program out;
    detail::lower_header(program.header, out);

    detail::LowererContext ctx;
    ctx.collect_patterns(program.globals);
    ctx.execute_block = [&ctx](const ast::Block& b, double& cur) { return detail::lower_block(b, ctx, cur); };

    // Lower global let bindings (patterns already collected above).
    ctx.push_scope();
    for (const auto& item : program.globals) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            if (const auto* let = std::get_if<ast::LetStatement>(&(*stmt_ptr)->kind)) {
                detail::lower_let_statement(*let, ctx);
            }
        }
    }

    for (const auto& track : program.tracks) {
        out.tracks.push_back(detail::lower_track_definition(track, ctx));
    }

    ctx.pop_scope();
    return out;
}

}  // namespace dsl::ir
