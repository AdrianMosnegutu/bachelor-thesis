#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "dsl/backend/midi_writer.hpp"
#include "dsl/core/errors/lexical_error.hpp"
#include "dsl/core/errors/lowerer_error.hpp"
#include "dsl/core/errors/syntax_error.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/semantic/analyzer.hpp"
#include "parser.hpp"

// Flex-generated globals (C interface)
extern FILE* yyin;

namespace {

namespace ast = dsl::ast;
namespace err = dsl::errors;

namespace fe = dsl::frontend;
namespace sem = dsl::semantic;
namespace ir = dsl::ir;
namespace be = dsl::backend;

using ProgramNodePtr = std::unique_ptr<ast::Program>;
using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;

const char* executable_name;

class ScannerInputGuard {
   public:
    explicit ScannerInputGuard(FILE* input) : previous_(yyin) { yyin = input; }

    ScannerInputGuard(const ScannerInputGuard&) = delete;
    ScannerInputGuard& operator=(const ScannerInputGuard&) = delete;

    ~ScannerInputGuard() { yyin = previous_; }

   private:
    FILE* previous_;
};

void print_usage() {
    std::cerr << "usage: " << executable_name << " <file.dsl>\n"
              << "       " << executable_name << " -          (read from stdin)\n";
}

ProgramNodePtr parse(const std::string& src_path, FILE* input) {
    ScannerInputGuard _(input);
    auto program = std::make_unique<ast::Program>();

    try {
        dsl::Location loc(&src_path);
        return fe::Parser(loc, *program).parse() == EXIT_SUCCESS ? std::move(program) : nullptr;
    } catch (const err::LexicalError& e) {
        std::cerr << e.format() << '\n';
        exit(EXIT_FAILURE);
    } catch (const err::SyntaxError& e) {
        std::cerr << e.format() << '\n';
        exit(EXIT_FAILURE);
    }
}

sem::AnalysisResult analyze(const ProgramNodePtr& program) {
    auto result = sem::analyze(*program);

    for (const auto& diagnostic : result.diagnostics()) {
        if (diagnostic.is_error()) {
            std::cerr << executable_name << ": " << diagnostic.message << '\n';
        }
    }

    if (result.has_errors()) {
        exit(EXIT_FAILURE);
    }

    return result;
}

void write(const std::string& out_path, const ir::Program& ir) {
    try {
        be::MidiWriter::write(ir, out_path);
        std::cout << "compile OK -> " << out_path << '\n';
    } catch (const std::exception& e) {
        std::cerr << executable_name << ": " << e.what() << '\n';
        exit(EXIT_FAILURE);
    }
}

std::string output_path(const char* src_path) {
    return std::filesystem::path(src_path).replace_extension(".mid").string();
}

}  // namespace

int main(const int argc, char* argv[]) {
    executable_name = argv[0];

    if (argc != 2) {
        print_usage();
        return EXIT_FAILURE;
    }

    const char* src_path = argv[1];
    const FilePtr src_file(std::fopen(src_path, "r"), &std::fclose);

    if (!src_file) {
        std::cerr << executable_name << ": cannot open '" << src_path << "'\n";
        return EXIT_FAILURE;
    }

    auto program = parse(src_path, src_file.get());
    auto analysis = analyze(program);

    try {
        write(output_path(src_path), ir::lower(analysis));
    } catch (const err::LowererError& e) {
        std::cerr << e.format() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
