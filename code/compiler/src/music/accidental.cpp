#include "dsl/music/accidental.hpp"

namespace dsl::music {

Accidental char_to_accidental(const char c) {
    switch (c) {
        case '#':
            return Accidental::Sharp;
        case 'b':
            return Accidental::Flat;
        default:
            return Accidental::Natural;
    }
}

}  // namespace dsl::music
