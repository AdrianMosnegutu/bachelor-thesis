#include <filesystem>
#include <iostream>
#include <memory>

#include "dsl/compiler.hpp"

using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;

std::string output_path(const char* src_path) {
    return std::filesystem::path(src_path).replace_extension(".mid").string();
}

int main(const int argc, char* argv[]) {
    const char* executable_name = argv[0];

    if (argc != 2) {
        std::cerr << "usage: " << executable_name << " <file.dsl>\n";
        std::cerr << "       " << executable_name << " -          (read from stdin)\n";
        return EXIT_FAILURE;
    }

    const char* src_path = argv[1];
    const bool is_using_stdin = std::string_view(src_path) == "-";
    FilePtr src_file(nullptr, &std::fclose);

    if (!is_using_stdin) {
        src_file.reset(std::fopen(src_path, "r"));
    }

    FILE* input = is_using_stdin ? stdin : src_file.get();
    if (input == nullptr) {
        std::cerr << executable_name << ": cannot open '" << src_path << "'\n";
        return EXIT_FAILURE;
    }

    const std::string source_name = is_using_stdin ? "<stdin>" : src_path;
    const std::string out_path = is_using_stdin ? "out.mid" : output_path(src_path);

    const auto result = dsl::compile(input, source_name, out_path);
    for (const auto& error : result.errors) {
        std::cerr << executable_name << ": " << dsl::to_string(error.stage) << ": " << error.message << '\n';
    }

    if (!result.ok()) {
        return EXIT_FAILURE;
    }

    std::cout << "compile OK -> " << out_path << '\n';
    return EXIT_SUCCESS;
}
