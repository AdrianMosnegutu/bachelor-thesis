#pragma once

#include "dsl/core/location.hpp"

namespace dsl::ast {

struct TempoDeclaration {
    int beats_per_minute{};
    Location location;
};

}  // namespace dsl::ast