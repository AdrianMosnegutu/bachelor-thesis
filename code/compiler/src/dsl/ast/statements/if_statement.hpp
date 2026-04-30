#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "dsl/ast/expression.hpp"

namespace dsl::ast {

struct Statement;

struct IfStatement {
    ExpressionPtr condition;
    std::vector<std::unique_ptr<Statement>> then_branch;
    std::optional<std::vector<std::unique_ptr<Statement>>> else_branch;
};

}  // namespace dsl::ast
