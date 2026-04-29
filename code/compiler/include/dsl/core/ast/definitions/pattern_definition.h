#pragma once

#include <string>
#include <vector>

#include "dsl/core/ast/statement.hpp"

namespace dsl::ast {

struct PatternDefinition {
    std::string name;
    std::vector<std::string> params;
    Block body;
    Location location;
};

}  // namespace dsl::ast