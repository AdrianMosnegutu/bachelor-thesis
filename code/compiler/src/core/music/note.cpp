#include "dsl/core/music/note.hpp"

namespace dsl::music {

namespace {

constexpr uint8_t NOTES_PER_OCTAVE = 12;
constexpr uint8_t OCTAVE_OFFSET = 2;

}  // namespace

uint8_t Note::midi_number() const {
    return NOTES_PER_OCTAVE * (octave + OCTAVE_OFFSET) + static_cast<uint8_t>(pitch) + static_cast<int8_t>(accidental);
}

}  // namespace dsl::music
