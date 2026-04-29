#pragma once

#include <vector>

#include "dsl/ast/decl.hpp"

namespace dsl::ast {

struct Program {
    Header header;
    std::vector<GlobalItem> globals;
    std::vector<TrackDeclaration> tracks;
};

}  // namespace dsl::ast
