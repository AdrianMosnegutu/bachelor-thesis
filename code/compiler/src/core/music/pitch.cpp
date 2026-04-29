#include "dsl/core/music/pitch.hpp"

namespace dsl::music {

Pitch letter_to_pitch(const char c) {
    switch (c) {
        case 'A':
            return Pitch::A;
        case 'B':
            return Pitch::B;
        case 'C':
            return Pitch::C;
        case 'D':
            return Pitch::D;
        case 'E':
            return Pitch::E;
        case 'F':
            return Pitch::F;
        case 'G':
            return Pitch::G;
        default:
            return Pitch::C;  // unreachable: regex only matches [A-G]
    }
}

}  // namespace dsl::music
