#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "scanner/scan_result.hpp"

namespace compiler::scanner::internal {

/**
 * @brief Minimal runtime state shared between Flex actions and the scan wrapper.
 */
class ScannerRuntime {
   public:
    /**
     * @brief Stores the semantic value of the current token.
     * @param lexeme Lexeme text to expose through `yylval`.
     */
    static void set_parser_value(std::string_view lexeme);

    /**
     * @brief Resets scanner state for a new scan.
     *
     * Clears the current token span, recorded lexical errors, and parser-facing
     * globals.
     */
    void reset();

    /**
     * @brief Updates the current token span from a matched lexeme.
     * @param lexeme Matched lexeme text.
     * @param ending_line 1-based line number reported by the lexer after matching
     * the lexeme.
     */
    void update_token_span(std::string_view lexeme, std::size_t ending_line);

    /**
     * @brief Records a lexical error for the current token span.
     * @param lexeme Lexeme that caused the error.
     * @param message Diagnostic message describing the error.
     */
    void report_lexical_error(std::string_view lexeme, std::string message);

    /**
     * @brief Copies the current token span into `yylloc`.
     */
    void sync_parser_location() const;

    /**
     * @brief Returns the span of the most recently matched token.
     * @return Current token span.
     */
    [[nodiscard]] SourceSpan token_span() const;

    /**
     * @brief Returns lexical errors collected during the current scan.
     * @return Recorded lexical errors.
     */
    [[nodiscard]] const std::vector<ScanError>& errors() const;

    /**
     * @brief Moves lexical errors collected during the current scan.
     * @return Recorded lexical errors.
     */
    [[nodiscard]] std::vector<ScanError> take_errors();

   private:
    std::size_t current_column_{1};
    SourceSpan token_span_{};
    std::vector<ScanError> errors_{};
};

/**
 * @brief Returns the process-local scanner runtime used by Flex actions.
 * @return Shared scanner runtime instance.
 */
ScannerRuntime& runtime();

/**
 * @brief Finalizes a regular token for parser consumption.
 * @param token Parser token code to return.
 * @param lexeme Matched lexeme text.
 * @return The provided parser token code.
 */
int make_parser_token(TokenType token, std::string_view lexeme);

/**
 * @brief Finalizes an error token and records its diagnostic.
 * @param lexeme Lexeme that triggered the lexical error.
 * @param message Diagnostic message describing the error.
 * @return `TokenType::Error`.
 */
int make_error_token(std::string_view lexeme, std::string message);

}  // namespace compiler::scanner::internal
