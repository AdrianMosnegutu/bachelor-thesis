#include "dsl/common/music/note.hpp"

namespace dsl::music {

uint8_t Note::midi_number() const {
    static constexpr uint8_t NOTES_PER_OCTAVE = 12;
    static constexpr uint8_t OCTAVE_OFFSET = 1;

    const auto pitch_value = static_cast<uint8_t>(pitch);
    const auto accidental_value = static_cast<uint8_t>(accidental);

    return NOTES_PER_OCTAVE * (octave + OCTAVE_OFFSET) + pitch_value + accidental_value;
}

}  // namespace dsl::music
