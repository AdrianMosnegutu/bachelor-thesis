#pragma once

#include "dsl/music/accidental.hpp"
#include "dsl/music/pitch.hpp"

namespace dsl::music {

struct PitchClass {
    Pitch pitch;
    Accidental accidental;
};

}  // namespace dsl::music
