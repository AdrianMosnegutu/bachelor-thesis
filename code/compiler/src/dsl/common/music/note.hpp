#pragma once

#include "accidental.hpp"
#include "pitch.hpp"

namespace dsl::music {

struct Note {
    Pitch pitch;
    Accidental accidental;
    uint8_t octave;  // 0 - 8

    [[nodiscard]] uint8_t midi_number() const;
};

}  // namespace dsl::music
