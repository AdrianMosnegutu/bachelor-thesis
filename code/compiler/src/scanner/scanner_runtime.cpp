#include "scanner/scanner_runtime.hpp"

#include <utility>

namespace compiler::scanner {

SemanticValue yylval{};
SourceSpan yylloc{};

namespace internal {

void ScannerRuntime::reset() {
    current_column_ = 1;
    token_span_ = {};
    errors_.clear();
    yylval = {};
    yylloc = {};
}

void ScannerRuntime::update_token_span(const std::string_view lexeme, const std::size_t ending_line) {
    std::size_t newline_count = 0;
    std::size_t trailing_column_width = 0;

    for (const char character : lexeme) {
        if (character == '\n') {
            ++newline_count;
            trailing_column_width = 0;
            continue;
        }

        ++trailing_column_width;
    }

    token_span_ = SourceSpan{
        .begin =
            SourceLocation{
                .line = ending_line - newline_count,
                .column = current_column_,
            },
        .end =
            SourceLocation{
                .line = ending_line,
                .column = newline_count == 0 ? current_column_ + lexeme.size() : trailing_column_width + 1,
            },
    };

    current_column_ = token_span_.end.column;
}

void ScannerRuntime::report_lexical_error(const std::string_view lexeme, std::string message) {
    errors_.push_back(ScanError{
        .span = token_span(),
        .lexeme = std::string{lexeme},
        .message = std::move(message),
    });
}

void ScannerRuntime::set_parser_value(const std::string_view lexeme) { yylval.lexeme = lexeme; }

void ScannerRuntime::sync_parser_location() const { yylloc = token_span(); }

SourceSpan ScannerRuntime::token_span() const { return token_span_; }

const std::vector<ScanError>& ScannerRuntime::errors() const { return errors_; }

std::vector<ScanError> ScannerRuntime::take_errors() { return std::move(errors_); }

ScannerRuntime& runtime() {
    static ScannerRuntime scanner_runtime{};
    return scanner_runtime;
}

int make_parser_token(const TokenType token, const std::string_view lexeme) {
    const auto& scanner = runtime();
    scanner.sync_parser_location();
    ScannerRuntime::set_parser_value(lexeme);
    return static_cast<int>(token);
}

int make_error_token(const std::string_view lexeme, std::string message) {
    auto& scanner = runtime();
    scanner.sync_parser_location();
    scanner.report_lexical_error(lexeme, std::move(message));
    ScannerRuntime::set_parser_value(lexeme);
    return static_cast<int>(TokenType::Error);
}

}  // namespace internal
}  // namespace compiler::scanner
