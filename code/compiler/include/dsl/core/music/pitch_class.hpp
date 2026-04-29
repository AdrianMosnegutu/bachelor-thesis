#pragma once

#include "dsl/core/music/accidental.hpp"
#include "dsl/core/music/pitch.hpp"

namespace dsl::music {

struct PitchClass {
    Pitch pitch;
    Accidental accidental;
};

}  // namespace dsl::music
