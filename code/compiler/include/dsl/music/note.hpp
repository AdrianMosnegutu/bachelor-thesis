#pragma once

#include "dsl/music/pitch_class.hpp"

namespace dsl::music {

struct Note : PitchClass {
    uint8_t octave;  // 0 - 8

    [[nodiscard]] uint8_t midi_number() const {
        static constexpr uint8_t NOTES_PER_OCTAVE = 12;
        static constexpr uint8_t OCTAVE_OFFSET = 2;

        return NOTES_PER_OCTAVE * (octave + OCTAVE_OFFSET) + static_cast<uint8_t>(pitch) +
               static_cast<int8_t>(accidental);
    }
};

}  // namespace dsl::music
