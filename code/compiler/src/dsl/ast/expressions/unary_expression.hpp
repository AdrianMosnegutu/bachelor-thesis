#pragma once

#include <memory>

#include "dsl/ast/operators.hpp"

namespace dsl::ast {

struct Expression;

struct UnaryExpression {
    UnaryOperator operation;
    std::unique_ptr<Expression> operand;
};

}  // namespace dsl::ast
