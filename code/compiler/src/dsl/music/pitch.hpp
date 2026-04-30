#pragma once

#include <cstdint>

namespace dsl::music {

enum class Pitch : uint8_t { C = 0, D = 2, E = 4, F = 5, G = 7, A = 9, B = 11 };

[[nodiscard]] Pitch letter_to_pitch(char c);

}  // namespace dsl::music
