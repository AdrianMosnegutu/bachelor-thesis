#pragma once

#include <variant>
#include <vector>

#include "dsl/common/ast/declarations.hpp"
#include "dsl/common/ast/definitions.hpp"
#include "dsl/common/ast/statements.hpp"

namespace dsl::ast {

using GlobalItem = std::variant<StatementPtr, PatternDefinition>;

struct Program {
    Header header;
    std::vector<GlobalItem> globals;
    std::vector<TrackDefinition> tracks;
};

}  // namespace dsl::ast
