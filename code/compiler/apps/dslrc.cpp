#include <cstdio>
#include <filesystem>
#include <iostream>
#include <memory>

#include "dsl/compiler.hpp"

using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;

void print_usage(const char* executable_name);
std::string output_path(const char* src_path);

int main(const int argc, char* argv[]) {
    const char* executable_name = argv[0];

    if (argc != 2) {
        print_usage(executable_name);
        return EXIT_FAILURE;
    }

    const char* src_path = argv[1];
    const bool is_using_stdin = std::string_view(src_path) == "-";

    const FilePtr src_file(is_using_stdin ? nullptr : std::fopen(src_path, "r"), &std::fclose);
    FILE* input = is_using_stdin ? stdin : src_file.get();

    if (!input) {
        std::cerr << executable_name << ": cannot open '" << src_path << "'\n";
        return EXIT_FAILURE;
    }

    const std::string source_name = is_using_stdin ? "<stdin>" : src_path;
    const std::string out_path = is_using_stdin ? "out.mid" : output_path(src_path);

    const auto result = dsl::compile(input, source_name, out_path);
    for (const auto& [stage, message] : result.get_diagnostics()) {
        std::cerr << executable_name << ": " << dsl::to_string(stage) << ": " << message << '\n';
    }

    if (!result.ok()) {
        return EXIT_FAILURE;
    }

    std::cout << "compile OK -> " << out_path << '\n';
    return EXIT_SUCCESS;
}

void print_usage(const char* executable_name) {
    std::cerr << "usage: " << executable_name << " <file.dsl>\n";
    std::cerr << "       " << executable_name << " -          (read from stdin)\n";
}

std::string output_path(const char* src_path) {
    return std::filesystem::path(src_path).replace_extension(".mid").string();
}
