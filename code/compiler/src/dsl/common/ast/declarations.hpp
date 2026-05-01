#pragma once

#include <optional>

#include "dsl/common/source/location.hpp"

namespace dsl::ast {

struct SignatureDeclaration {
    int beats{};
    int unit{};
    source::Location location;
};

struct TempoDeclaration {
    int beats_per_minute{};
    source::Location location;
};

struct Header {
    std::optional<TempoDeclaration> tempo;
    std::optional<SignatureDeclaration> signature;
};

}  // namespace dsl::ast
