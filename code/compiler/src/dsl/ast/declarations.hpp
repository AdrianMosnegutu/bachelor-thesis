#pragma once
#include "dsl/source/location.hpp"

namespace dsl::ast {

struct SignatureDeclaration {
    int beats{};
    int unit{};
    Location location;
};

struct TempoDeclaration {
    int beats_per_minute{};
    Location location;
};

struct Header {
    std::optional<TempoDeclaration> tempo;
    std::optional<SignatureDeclaration> signature;
};

}  // namespace dsl::ast