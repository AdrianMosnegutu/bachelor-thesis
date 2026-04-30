#pragma once

#include "dsl/source/location.hpp"

namespace dsl::ast {

struct TempoDeclaration {
    int beats_per_minute{};
    Location location;
};

}  // namespace dsl::ast
