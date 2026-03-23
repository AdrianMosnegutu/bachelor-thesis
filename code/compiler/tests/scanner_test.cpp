#include "scanner/scanner.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace compiler::scanner {
namespace {

std::string ReadFixture(const std::string& file_name) {
    const std::filesystem::path path = std::filesystem::path{COMPILER_TEST_DATA_DIR} / file_name;
    std::ifstream input{path};
    EXPECT_TRUE(input.is_open()) << path;
    return std::string{std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
}

const ScannedToken* FindToken(const std::vector<ScannedToken>& tokens, const TokenType kind,
                              const std::string_view lexeme) {
    const auto it = std::ranges::find_if(
        tokens, [kind, lexeme](const ScannedToken& token) { return token.kind == kind && token.lexeme == lexeme; });
    return it == tokens.end() ? nullptr : &*it;
}

class ScannerKeywordsAndIdentifiersTest : public ::testing::Test {};

class ScannerConstantsTest : public ::testing::Test {};

class ScannerSeparatorsAndOperatorsTest : public ::testing::Test {};

class ScannerLexerErrorsTest : public ::testing::Test {};

TEST_F(ScannerKeywordsAndIdentifiersTest, RecognizesKeywordsAndIdentifiersFromValidProgram) {
    const auto [tokens, errors] = scan(ReadFixture("valid.mus"));

    ASSERT_FALSE(tokens.empty());
    EXPECT_TRUE(errors.empty());

    ASSERT_NE(FindToken(tokens, TokenType::KeywordTempo, "tempo"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordTimeSignature, "time_signature"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordKey, "key"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordPattern, "pattern"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordPlay, "play"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordFrom, "from"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordLoop, "loop"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordNum, "num"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordNote, "note"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordChord, "chord"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordIf, "if"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordElse, "else"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::KeywordUsing, "using"), nullptr);

    ASSERT_NE(FindToken(tokens, TokenType::Identifier, "intro_1"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::Identifier, "$i"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::Identifier, "$offset"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::Identifier, "saw"), nullptr);
}

TEST_F(ScannerConstantsTest, RecognizesNoteAndNumericConstantsFromValidProgram) {
    const auto [tokens, errors] = scan(ReadFixture("valid.mus"));

    ASSERT_FALSE(tokens.empty());
    EXPECT_TRUE(errors.empty());

    ASSERT_NE(FindToken(tokens, TokenType::ConstantNumber, "120"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNumber, "4"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNumber, "2"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNumber, "2.5"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNumber, "0"), nullptr);

    ASSERT_NE(FindToken(tokens, TokenType::ConstantNote, "D#"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNote, "A3"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNote, "A2"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNote, "B3"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNote, "C3"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNote, "Gb3"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::ConstantNote, "B5"), nullptr);
}

TEST_F(ScannerSeparatorsAndOperatorsTest, RecognizesSeparatorsAndOperatorsFromValidProgram) {
    const auto [tokens, errors] = scan(ReadFixture("valid.mus"));

    ASSERT_FALSE(tokens.empty());
    EXPECT_TRUE(errors.empty());

    ASSERT_NE(FindToken(tokens, TokenType::SeparatorSemicolon, ";"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::SeparatorComma, ","), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::SeparatorLeftParenthesis, "("), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::SeparatorRightParenthesis, ")"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::SeparatorLeftBrace, "{"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::SeparatorRightBrace, "}"), nullptr);

    ASSERT_NE(FindToken(tokens, TokenType::OperatorAssign, "="), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::OperatorPlus, "+"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::OperatorMultiply, "*"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::OperatorDivide, "/"), nullptr);
    ASSERT_NE(FindToken(tokens, TokenType::OperatorModulo, "%"), nullptr);
}

TEST_F(ScannerLexerErrorsTest, ReportsLexicalErrorsFromInvalidProgramAndContinuesScanning) {
    const auto [tokens, errors] = scan(ReadFixture("scanner-err.mus"));

    ASSERT_FALSE(tokens.empty());
    ASSERT_EQ(errors.size(), 9U);

    EXPECT_EQ(errors[0].lexeme, "!");
    EXPECT_EQ(errors[0].span.begin.line, 1U);
    EXPECT_EQ(errors[0].span.begin.column, 9U);

    EXPECT_EQ(errors[1].lexeme, "@");
    EXPECT_EQ(errors[1].span.begin.line, 2U);
    EXPECT_EQ(errors[1].span.begin.column, 10U);

    EXPECT_EQ(errors[2].lexeme, "!");
    EXPECT_EQ(errors[2].span.begin.line, 2U);
    EXPECT_EQ(errors[2].span.begin.column, 11U);

    EXPECT_EQ(errors[3].lexeme, "#");
    EXPECT_EQ(errors[3].span.begin.line, 2U);
    EXPECT_EQ(errors[3].span.begin.column, 12U);

    EXPECT_EQ(errors[4].lexeme, "$");
    EXPECT_EQ(errors[4].span.begin.line, 12U);
    EXPECT_EQ(errors[4].span.begin.column, 23U);

    EXPECT_EQ(errors[5].lexeme, "#");
    EXPECT_EQ(errors[5].span.begin.line, 12U);
    EXPECT_EQ(errors[5].span.begin.column, 24U);

    EXPECT_EQ(errors[6].lexeme, "$");
    EXPECT_EQ(errors[6].span.begin.line, 15U);
    EXPECT_EQ(errors[6].span.begin.column, 9U);

    EXPECT_EQ(errors[7].lexeme, "$");
    EXPECT_EQ(errors[7].span.begin.line, 15U);
    EXPECT_EQ(errors[7].span.begin.column, 10U);

    EXPECT_EQ(errors[8].lexeme, "!");
    EXPECT_EQ(errors[8].span.begin.line, 25U);
    EXPECT_EQ(errors[8].span.begin.column, 6U);

    EXPECT_EQ(tokens.front().kind, TokenType::KeywordTempo);
    EXPECT_EQ(tokens.front().lexeme, "tempo");
    EXPECT_EQ(tokens.back().kind, TokenType::SeparatorSemicolon);
    EXPECT_EQ(tokens.back().lexeme, ";");
}

TEST_F(ScannerConstantsTest, RejectsInvalidAccidentalsAsNotes) {
    const auto [tokens, errors] = scan("play B# from 0;\nplay Cb from 1;\nplay Fb from 2;\nplay E# from 3;");

    EXPECT_EQ(FindToken(tokens, TokenType::ConstantNote, "B#"), nullptr);
    EXPECT_EQ(FindToken(tokens, TokenType::ConstantNote, "Cb"), nullptr);
    EXPECT_EQ(FindToken(tokens, TokenType::ConstantNote, "Fb"), nullptr);
    EXPECT_EQ(FindToken(tokens, TokenType::ConstantNote, "E#"), nullptr);

    ASSERT_EQ(errors.size(), 4U);
    EXPECT_EQ(errors[0].lexeme, "B#");
    EXPECT_EQ(errors[0].message, "Invalid note accidental.");
    EXPECT_EQ(errors[1].lexeme, "Cb");
    EXPECT_EQ(errors[2].lexeme, "Fb");
    EXPECT_EQ(errors[3].lexeme, "E#");
}

}  // namespace
}  // namespace compiler::scanner
