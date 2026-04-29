#pragma once

#include <memory>
#include <vector>

#include "dsl/core/ast/expression.hpp"

namespace dsl::ast {

struct Statement;

struct ForStatement {
    std::unique_ptr<Statement> init;
    ExpressionPtr condition;  // null for `for (;;)`
    std::unique_ptr<Statement> step;
    std::vector<std::unique_ptr<Statement>> body;
};

}  // namespace dsl::ast