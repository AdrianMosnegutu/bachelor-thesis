#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "dsl/compiler.hpp"

namespace {

namespace fs = std::filesystem;
using FilePtr = std::unique_ptr<FILE, int (*)(FILE*)>;

fs::path fixture(const std::string& name) {
    return fs::path(GOLDEN_FIXTURES_DIR) / name;
}

fs::path temp_midi(const std::string& stem) {
    return fs::temp_directory_path() / (stem + ".mid");
}

dsl::CompileResult compile_fixture(const std::string& fixture_name, const fs::path& output) {
    const fs::path src = fixture(fixture_name);
    const FilePtr input(std::fopen(src.string().c_str(), "r"), &std::fclose);
    EXPECT_NE(input, nullptr) << "Could not open fixture: " << src;
    return dsl::compile(input.get(), src.string(), output.string());
}

std::vector<uint8_t> read_bytes(const fs::path& path) {
    std::ifstream f(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

bool contains_note_on(const std::vector<uint8_t>& bytes, uint8_t note) {
    for (std::size_t i = 0; i + 1 < bytes.size(); ++i) {
        if ((bytes[i] & 0xF0) == 0x90 && bytes[i + 1] == note) return true;
    }
    return false;
}

int count_diagnostics_at_stage(const dsl::CompileResult& result, dsl::DiagnosticStage stage) {
    int n = 0;
    for (const auto& d : result.get_diagnostics()) {
        if (d.stage == stage) ++n;
    }
    return n;
}

}  // namespace

TEST(Golden, HappyPathCompilesAndProducesValidMidi) {
    const auto out = temp_midi("golden_happy");
    fs::remove(out);

    const auto result = compile_fixture("happy_path.dsl", out);

    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(result.get_diagnostics().empty());
    ASSERT_TRUE(fs::exists(out));

    const auto bytes = read_bytes(out);
    ASSERT_GE(bytes.size(), 14u);

    // MThd header
    EXPECT_EQ(bytes[0], 'M'); EXPECT_EQ(bytes[1], 'T');
    EXPECT_EQ(bytes[2], 'h'); EXPECT_EQ(bytes[3], 'd');

    // Format 1 (big-endian at bytes 8-9)
    EXPECT_EQ(bytes[8], 0x00); EXPECT_EQ(bytes[9], 0x01);

    // 3 tracks: tempo + melody + bass (big-endian at bytes 10-11)
    EXPECT_EQ(bytes[10], 0x00); EXPECT_EQ(bytes[11], 0x03);

    // Notes from the DSL program: motif plays A4 and C5, bass_line plays A2 and E3
    EXPECT_TRUE(contains_note_on(bytes, 69));  // A4
    EXPECT_TRUE(contains_note_on(bytes, 72));  // C5
    EXPECT_TRUE(contains_note_on(bytes, 45));  // A2
    EXPECT_TRUE(contains_note_on(bytes, 52));  // E3

    fs::remove(out);
}

TEST(Golden, ParseErrorsAreCollected) {
    const auto out = temp_midi("golden_parse");
    fs::remove(out);

    const auto result = compile_fixture("parse_errors.dsl", out);

    EXPECT_FALSE(result.ok());
    EXPECT_GE(count_diagnostics_at_stage(result, dsl::DiagnosticStage::Parsing), 2);
    EXPECT_FALSE(fs::exists(out));

    fs::remove(out);
}

TEST(Golden, SemanticErrorsAreCollectedAndGateLowering) {
    const auto out = temp_midi("golden_semantic");
    fs::remove(out);

    const auto result = compile_fixture("semantic_errors.dsl", out);

    EXPECT_FALSE(result.ok());
    EXPECT_GE(count_diagnostics_at_stage(result, dsl::DiagnosticStage::Semantic), 3);
    EXPECT_EQ(count_diagnostics_at_stage(result, dsl::DiagnosticStage::Lowering), 0);
    EXPECT_FALSE(fs::exists(out));

    fs::remove(out);
}

TEST(Golden, LoweringErrorsAreCollectedAcrossTracks) {
    const auto out = temp_midi("golden_lowering");
    fs::remove(out);

    const auto result = compile_fixture("lowering_errors.dsl", out);

    EXPECT_FALSE(result.ok());
    EXPECT_GE(count_diagnostics_at_stage(result, dsl::DiagnosticStage::Lowering), 2);
    EXPECT_EQ(count_diagnostics_at_stage(result, dsl::DiagnosticStage::Semantic), 0);
    EXPECT_FALSE(fs::exists(out));

    fs::remove(out);
}
