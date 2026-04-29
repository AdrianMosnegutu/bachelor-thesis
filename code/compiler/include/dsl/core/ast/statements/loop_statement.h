#pragma once

#include <memory>
#include <vector>

#include "dsl/core/ast/expression.hpp"

namespace dsl::ast {

struct Statement;

struct LoopStatement {
    ExpressionPtr count;
    std::vector<std::unique_ptr<Statement>> body;
};

}  // namespace dsl::ast