#pragma once

#include <string>

#include "dsl/ast/expression.hpp"

namespace dsl::ast {

struct LetStatement {
    std::string name;
    ExpressionPtr value;
};

}  // namespace dsl::ast
