#pragma once

#include <cstddef>

namespace compiler::scanner {

/**
 * @brief One source position in 1-based line and column coordinates.
 */
struct SourceLocation {
    std::size_t line{1};
    std::size_t column{1};
};

/**
 * @brief Half-open source span covering a matched lexeme.
 *
 * `begin` points to the first character of the lexeme and `end` points one past the last character.
 */
struct SourceSpan {
    SourceLocation begin{};
    SourceLocation end{};
};

}  // namespace compiler::scanner
