#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "dsl/compiler.hpp"

namespace {

using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;
namespace fs = std::filesystem;

fs::path test_path(const std::string& filename) { return fs::temp_directory_path() / filename; }

void write_text_file(const fs::path& path, const std::string& contents) {
    std::ofstream file(path);
    file << contents;
}

}  // namespace

TEST(CompilerApi, CompilesSourceFileToMidi) {
    const fs::path source_path = test_path("dsl_compiler_api_valid.dsl");
    const fs::path output_path = test_path("dsl_compiler_api_valid.mid");
    fs::remove(output_path);
    write_text_file(source_path, "track { play A4; }");

    const FilePtr input(std::fopen(source_path.string().c_str(), "r"), &std::fclose);
    ASSERT_NE(input, nullptr);

    const auto result = dsl::compile(input.get(), source_path.string(), output_path.string());

    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.get_diagnostics().empty());
    ASSERT_TRUE(fs::exists(output_path));
    EXPECT_GT(fs::file_size(output_path), 0u);

    fs::remove(source_path);
    fs::remove(output_path);
}

TEST(CompilerApi, ReportsSemanticErrorsWithoutWritingMidi) {
    const fs::path source_path = test_path("dsl_compiler_api_invalid.dsl");
    const fs::path output_path = test_path("dsl_compiler_api_invalid.mid");
    fs::remove(output_path);
    write_text_file(source_path, "track { play missing; }");

    const FilePtr input(std::fopen(source_path.string().c_str(), "r"), &std::fclose);
    ASSERT_NE(input, nullptr);

    const auto result = dsl::compile(input.get(), source_path.string(), output_path.string());

    ASSERT_FALSE(result.ok());
    ASSERT_FALSE(result.get_diagnostics().empty());
    EXPECT_EQ(result.get_diagnostics().front().stage, dsl::CompileStage::Semantic);
    EXPECT_FALSE(fs::exists(output_path));

    fs::remove(source_path);
}
