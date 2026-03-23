#pragma once

#include <string>
#include <vector>

#include "scanner/source_location.hpp"
#include "scanner/token.hpp"

namespace compiler::scanner {

/**
 * @brief Lexical error captured during scanning.
 *
 * Scanning continues after the error is recorded.
 */
struct ScanError {
    SourceSpan span{};
    std::string lexeme{};
    std::string message{};
};

/**
 * @brief Result of scanning a complete input string.
 */
struct ScanResult {
    std::vector<ScannedToken> tokens{};
    std::vector<ScanError> errors{};
};

}  // namespace compiler::scanner
