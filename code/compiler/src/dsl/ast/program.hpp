#pragma once

#include <variant>
#include <vector>

#include "declarations.hpp"
#include "definitions.hpp"
#include "statements.hpp"

namespace dsl::ast {

using GlobalItem = std::variant<StatementPtr, PatternDefinition>;

struct Program {
    Header header;
    std::vector<GlobalItem> globals;
    std::vector<TrackDefinition> tracks;
};

}  // namespace dsl::ast
