#pragma once

#include <memory>

namespace dsl::ast {

struct Expression;

struct TernaryExpression {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> then_expression;
    std::unique_ptr<Expression> else_expression;
};

}  // namespace dsl::ast
