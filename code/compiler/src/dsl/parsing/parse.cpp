#include "dsl/parsing/parse.hpp"

#include <cstdlib>
#include <exception>
#include <utility>

#include "dsl/common/ast/program.hpp"
#include "dsl/common/source/location.hpp"
#include "parser.hpp"

extern FILE* yyin;
void yyrestart(FILE* input_file);

struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void scanner_reset();
void scanner_set_diagnostics(dsl::DiagnosticsEngine* diagnostics);

namespace dsl::parsing {

namespace {

class ScannerInputGuard {
   public:
    explicit ScannerInputGuard(FILE* input, DiagnosticsEngine& diagnostics) : previous_(yyin) {
        scanner_reset();
        scanner_set_diagnostics(&diagnostics);
        yyin = input;
        yyrestart(yyin);
    }

    ScannerInputGuard(const ScannerInputGuard&) = delete;
    ScannerInputGuard& operator=(const ScannerInputGuard&) = delete;

    ~ScannerInputGuard() {
        yyin = previous_;
        yyrestart(yyin);
        scanner_set_diagnostics(nullptr);
        scanner_reset();
    }

   private:
    FILE* previous_;
};

class ScannerBufferGuard {
   public:
    explicit ScannerBufferGuard(const std::string& source, DiagnosticsEngine& diagnostics)
        : buffer_(yy_scan_string(source.c_str())) {
        scanner_reset();
        scanner_set_diagnostics(&diagnostics);
    }

    ScannerBufferGuard(const ScannerBufferGuard&) = delete;
    ScannerBufferGuard& operator=(const ScannerBufferGuard&) = delete;

    ~ScannerBufferGuard() {
        yy_delete_buffer(buffer_);
        scanner_set_diagnostics(nullptr);
        scanner_reset();
    }

   private:
    YY_BUFFER_STATE buffer_;
};

ParseResult parse_current_input(const std::string&, DiagnosticsEngine& diagnostics) {
    try {
        auto program = std::make_unique<ast::Program>();
        source::Location loc;

        const int parse_exit_code = dsl::parsing::detail::Parser(loc, diagnostics, *program).parse();
        const bool has_parse_errors = diagnostics.has_errors(DiagnosticStage::Parsing);

        if (parse_exit_code == EXIT_FAILURE && !has_parse_errors) {
            diagnostics.report(DiagnosticStage::Parsing,
                               DiagnosticSeverity::Error,
                               "parser returned a non-zero status");
        }

        return ParseResult(has_parse_errors ? nullptr : std::move(program));
    } catch (const std::exception& error) {
        diagnostics.report(DiagnosticStage::Parsing, DiagnosticSeverity::Error, error.what());
        return ParseResult(nullptr);
    }
}

}  // namespace

ParseResult parse_stream(FILE* input, const std::string& source_name, DiagnosticsEngine& diagnostics) {
    ScannerInputGuard guard(input, diagnostics);
    return parse_current_input(source_name, diagnostics);
}

ParseResult parse_source(const std::string& source, const std::string& source_name, DiagnosticsEngine& diagnostics) {
    ScannerBufferGuard guard(source, diagnostics);
    return parse_current_input(source_name, diagnostics);
}

}  // namespace dsl::parsing
