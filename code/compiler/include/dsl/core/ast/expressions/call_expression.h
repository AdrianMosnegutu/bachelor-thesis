#pragma once

#include <string>
#include <vector>

namespace dsl::ast {

struct Expression;

struct CallExpression {
    std::string callee;
    std::vector<std::unique_ptr<Expression>> arguments;
};

}  // namespace dsl::ast