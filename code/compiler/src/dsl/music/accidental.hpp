#pragma once

#include <cstdint>

namespace dsl::music {

enum class Accidental : int8_t { Flat = -1, Natural = 0, Sharp = 1 };

[[nodiscard]] Accidental char_to_accidental(char c);

}  // namespace dsl::music
