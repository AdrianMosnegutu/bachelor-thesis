#include "dsl/frontend/parse.hpp"

#include <cstdlib>
#include <utility>

#include "dsl/ast/program.hpp"
#include "dsl/errors/lexical_error.hpp"
#include "dsl/errors/syntax_error.hpp"
#include "dsl/source/location.hpp"
#include "parser.hpp"

extern FILE* yyin;

struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void scanner_reset();

namespace dsl::frontend {

ParseResult::ParseResult() = default;

ParseResult::ParseResult(std::unique_ptr<ast::Program> program, std::vector<std::string> errors)
    : program(std::move(program)), errors(std::move(errors)) {}

ParseResult::ParseResult(ParseResult&&) noexcept = default;
ParseResult& ParseResult::operator=(ParseResult&&) noexcept = default;
ParseResult::~ParseResult() = default;

namespace {

class ScannerInputGuard {
   public:
    explicit ScannerInputGuard(FILE* input) : previous_(yyin) {
        scanner_reset();
        yyin = input;
    }

    ScannerInputGuard(const ScannerInputGuard&) = delete;
    ScannerInputGuard& operator=(const ScannerInputGuard&) = delete;

    ~ScannerInputGuard() {
        yyin = previous_;
        scanner_reset();
    }

   private:
    FILE* previous_;
};

class ScannerBufferGuard {
   public:
    explicit ScannerBufferGuard(const std::string& source) : buffer_(yy_scan_string(source.c_str())) {
        scanner_reset();
    }

    ScannerBufferGuard(const ScannerBufferGuard&) = delete;
    ScannerBufferGuard& operator=(const ScannerBufferGuard&) = delete;

    ~ScannerBufferGuard() {
        yy_delete_buffer(buffer_);
        scanner_reset();
    }

   private:
    YY_BUFFER_STATE buffer_;
};

ParseResult parse_current_input(const std::string& source_name) {
    auto program = std::make_unique<ast::Program>();

    try {
        static_cast<void>(source_name);
        Location loc;
        if (Parser(loc, *program).parse() == EXIT_SUCCESS) {
            return {std::move(program), {}};
        }

        return {nullptr, {"parser returned a non-zero status"}};
    } catch (const errors::LexicalError& error) {
        return {nullptr, {error.format()}};
    } catch (const errors::SyntaxError& error) {
        return {nullptr, {error.format()}};
    }
}

}  // namespace

ParseResult parse_stream(FILE* input, const std::string& source_name) {
    ScannerInputGuard guard(input);
    return parse_current_input(source_name);
}

ParseResult parse_source(const std::string& source, const std::string& source_name) {
    ScannerBufferGuard guard(source);
    return parse_current_input(source_name);
}

}  // namespace dsl::frontend
