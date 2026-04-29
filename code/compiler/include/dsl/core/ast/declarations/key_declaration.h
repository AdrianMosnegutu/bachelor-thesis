#pragma once

#include "dsl/core/location.hpp"
#include "dsl/core/music/key_mode.hpp"
#include "dsl/core/music/pitch_class.hpp"

namespace dsl::ast {

struct KeyDeclaration {
    music::PitchClass pitch{};
    music::KeyMode mode{};
    Location location;
};

}  // namespace dsl::ast