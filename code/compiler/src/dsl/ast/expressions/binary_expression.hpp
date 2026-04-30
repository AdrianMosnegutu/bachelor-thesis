#pragma once

#include <memory>

#include "dsl/ast/operators.hpp"

namespace dsl::ast {

struct Expression;

struct BinaryExpression {
    BinaryOperator operation;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
};

}  // namespace dsl::ast
