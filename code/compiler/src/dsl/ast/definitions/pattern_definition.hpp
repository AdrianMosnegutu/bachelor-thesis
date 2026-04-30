#pragma once

#include <string>
#include <vector>

#include "dsl/ast/statement.hpp"

namespace dsl::ast {

struct PatternDefinition {
    std::string name;
    std::vector<std::string> params;
    Block body;
    Location location;
};

}  // namespace dsl::ast
