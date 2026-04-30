#pragma once

#include "dsl/source/location.hpp"

namespace dsl::ast {

struct SignatureDeclaration {
    int beats{};
    int unit{};
    Location location;
};

}  // namespace dsl::ast
