#pragma once

#include <memory>

namespace dsl::ast {

struct Expression;

struct ParenthesisedExpression {
    std::unique_ptr<Expression> inner;
};

}  // namespace dsl::ast