#pragma once

#include <string_view>

#include "scanner/scan_result.hpp"

namespace compiler::scanner {

/**
 * @brief Scans a source string into parser-facing tokens and lexical errors.
 * @param source Source text to scan.
 * @return All produced tokens and recorded lexical errors.
 * @throws std::runtime_error If scanner support was built without Flex.
 */
ScanResult scan(std::string_view source);

/**
 * @brief Parser-facing semantic value filled by the latest token action.
 */
extern SemanticValue yylval;

/**
 * @brief Parser-facing source location filled by the latest token action.
 */
extern SourceSpan yylloc;

}  // namespace compiler::scanner
