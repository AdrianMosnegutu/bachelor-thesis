#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser.hpp"

// -- Flex interface --------------------------------------------------------
struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
extern int yylineno;
void scanner_reset();

// -- Aliases ---------------------------------------------------------------
using P = dsl::frontend::detail::Parser;
using SK = P::symbol_kind_type;
using S = P::symbol_kind;
using dsl::source::Location;

using dsl::music::Accidental;
using dsl::music::DrumNote;
using dsl::music::Instrument;
using dsl::music::Note;
using dsl::music::Pitch;

// -- Test helpers ----------------------------------------------------------
namespace {

struct ScanGuard {
    YY_BUFFER_STATE buf;
    explicit ScanGuard(const std::string& src) {
        scanner_reset();
        buf = yy_scan_string(src.c_str());
    }
    ~ScanGuard() {
        yy_delete_buffer(buf);
        scanner_reset();
    }
    ScanGuard(const ScanGuard&) = delete;
    ScanGuard& operator=(const ScanGuard&) = delete;
};

std::vector<SK> scan_kinds(const std::string& src) {
    ScanGuard guard(src);
    std::vector<SK> kinds;
    Location loc;
    while (true) {
        auto sym = yylex(loc);
        if (sym.kind() == S::S_YYEOF) break;
        kinds.push_back(sym.kind());
    }
    return kinds;
}

struct TokenInfo {
    [[maybe_unused]] SK kind;
    Location loc;
};

std::vector<TokenInfo> scan_with_loc(const std::string& src) {
    ScanGuard guard(src);
    std::vector<TokenInfo> tokens;
    Location loc;
    while (true) {
        auto sym = yylex(loc);
        if (sym.kind() == S::S_YYEOF) break;
        tokens.push_back({sym.kind(), loc});
    }
    return tokens;
}

template <typename T>
T scan_as(const std::string& src) {
    ScanGuard guard(src);
    Location loc;
    return yylex(loc).value.as<T>();
}

}  // namespace

// ===========================================================================
// Keywords
// ===========================================================================

TEST(Scanner, AllKeywords) {
    const auto tokens = scan_kinds("tempo signature key track pattern play for loop if else let using from rest");
    const std::vector expected = {
        S::S_TEMPO,
        S::S_SIGNATURE,
        S::S_KEY,
        S::S_TRACK,
        S::S_PATTERN,
        S::S_PLAY,
        S::S_FOR,
        S::S_LOOP,
        S::S_IF,
        S::S_ELSE,
        S::S_LET,
        S::S_USING,
        S::S_FROM,
        S::S_REST,
    };
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]) << "at index " << i;
}

TEST(Scanner, KeywordsNotConfusedWithNotesOrPitches) {
    EXPECT_EQ(scan_kinds("from")[0], S::S_FROM);
    EXPECT_EQ(scan_kinds("else")[0], S::S_ELSE);
    EXPECT_EQ(scan_kinds("for")[0], S::S_FOR);
    EXPECT_EQ(scan_kinds("key")[0], S::S_KEY);
    EXPECT_EQ(scan_kinds("loop")[0], S::S_LOOP);
}

TEST(Scanner, KeywordIdentifierDisambiguation) {
    // Identifiers starting with keyword prefixes must remain identifiers.
    EXPECT_EQ(scan_kinds("tempo2")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("signatures")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("tracking")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("looper")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("keyed")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("majoring")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("minors")[0], S::S_IDENT);
}

// ===========================================================================
// Float literals
// ===========================================================================

TEST(Scanner, FloatLiteral) {
    EXPECT_EQ(scan_kinds("0.5")[0], S::S_FLOAT_LIT);
    EXPECT_DOUBLE_EQ(scan_as<double>("0.5"), 0.5);
    EXPECT_DOUBLE_EQ(scan_as<double>("3.14"), 3.14);
    EXPECT_DOUBLE_EQ(scan_as<double>("1.0"), 1.0);
}

TEST(Scanner, FloatNotInt) {
    const auto tokens = scan_kinds("1.5");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], S::S_FLOAT_LIT);
}

TEST(Scanner, FloatLiteralOutOfRangeThrows) {
    const std::string too_large = std::string(400, '9') + ".0";
    EXPECT_THROW(scan_kinds(too_large), std::runtime_error);
}

// ===========================================================================
// Integer literals
// ===========================================================================

TEST(Scanner, IntLiteral) {
    EXPECT_EQ(scan_kinds("0")[0], S::S_INT_LIT);
    EXPECT_EQ(scan_as<int>("0"), 0);
    EXPECT_EQ(scan_as<int>("130"), 130);
    EXPECT_EQ(scan_as<int>("2345"), 2345);
}

TEST(Scanner, IntLiteralOutOfRangeThrows) {
    const std::string too_large = std::to_string(static_cast<long long>(std::numeric_limits<int>::max()) + 1);
    EXPECT_THROW(scan_kinds(too_large), std::runtime_error);
}

// ===========================================================================
// Boolean literals
// ===========================================================================

TEST(Scanner, BooleanLiteral) {
    EXPECT_EQ(scan_kinds("true")[0], S::S_BOOL_LIT);
    EXPECT_EQ(scan_kinds("false")[0], S::S_BOOL_LIT);
    EXPECT_TRUE(scan_as<bool>("true"));
    EXPECT_FALSE(scan_as<bool>("false"));
}

TEST(Scanner, BooleanLiteralsVsIdentsDisambiguation) {
    EXPECT_EQ(scan_kinds("True")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("TRUE")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("truefalse")[0], S::S_IDENT);
    EXPECT_EQ(scan_kinds("true1")[0], S::S_IDENT);
}

// ===========================================================================
// Instruments
// ===========================================================================

TEST(Scanner, Instruments) {
    const auto tokens = scan_kinds("piano guitar bass violin drums");
    ASSERT_EQ(tokens.size(), 5u);
    for (const auto& token : tokens) EXPECT_EQ(token, S::S_INSTRUMENT_LIT);

    EXPECT_EQ(scan_as<Instrument>("piano"), Instrument::Piano);
    EXPECT_EQ(scan_as<Instrument>("guitar"), Instrument::Guitar);
    EXPECT_EQ(scan_as<Instrument>("bass"), Instrument::Bass);
    EXPECT_EQ(scan_as<Instrument>("violin"), Instrument::Violin);
    EXPECT_EQ(scan_as<Instrument>("drums"), Instrument::Drums);
}

// ===========================================================================
// Drum notes
// ===========================================================================

TEST(Scanner, DrumNotes) {
    const auto tokens = scan_kinds("kick snare hihat crash ride");
    ASSERT_EQ(tokens.size(), 5u);
    for (const auto& token : tokens) EXPECT_EQ(token, S::S_DRUM_NOTE_LIT);

    EXPECT_EQ(scan_as<DrumNote>("kick"), DrumNote::Kick);
    EXPECT_EQ(scan_as<DrumNote>("snare"), DrumNote::Snare);
    EXPECT_EQ(scan_as<DrumNote>("hihat"), DrumNote::Hihat);
    EXPECT_EQ(scan_as<DrumNote>("crash"), DrumNote::Crash);
    EXPECT_EQ(scan_as<DrumNote>("ride"), DrumNote::Ride);
}

// ===========================================================================
// Note literals (pitched, with octave)
// ===========================================================================

TEST(Scanner, NoteNatural) {
    EXPECT_EQ(scan_kinds("E4")[0], S::S_NOTE_LIT);
    const auto e4 = scan_as<Note>("E4");
    EXPECT_EQ(e4.pitch, Pitch::E);
    EXPECT_EQ(e4.accidental, Accidental::Natural);
    EXPECT_EQ(e4.octave, 4);

    const auto a0 = scan_as<Note>("A0");
    EXPECT_EQ(a0.pitch, Pitch::A);
    EXPECT_EQ(a0.accidental, Accidental::Natural);
    EXPECT_EQ(a0.octave, 0);

    const auto g8 = scan_as<Note>("G8");
    EXPECT_EQ(g8.pitch, Pitch::G);
    EXPECT_EQ(g8.accidental, Accidental::Natural);
    EXPECT_EQ(g8.octave, 8);
}

TEST(Scanner, NoteSharp) {
    EXPECT_EQ(scan_kinds("F#4")[0], S::S_NOTE_LIT);
    const auto fs4 = scan_as<Note>("F#4");
    EXPECT_EQ(fs4.pitch, Pitch::F);
    EXPECT_EQ(fs4.accidental, Accidental::Sharp);
    EXPECT_EQ(fs4.octave, 4);

    const auto cs5 = scan_as<Note>("C#5");
    EXPECT_EQ(cs5.pitch, Pitch::C);
    EXPECT_EQ(cs5.accidental, Accidental::Sharp);
    EXPECT_EQ(cs5.octave, 5);
}

TEST(Scanner, NoteFlat) {
    EXPECT_EQ(scan_kinds("Bb3")[0], S::S_NOTE_LIT);
    const auto bb3 = scan_as<Note>("Bb3");
    EXPECT_EQ(bb3.pitch, Pitch::B);
    EXPECT_EQ(bb3.accidental, Accidental::Flat);
    EXPECT_EQ(bb3.octave, 3);

    const auto eb2 = scan_as<Note>("Eb2");
    EXPECT_EQ(eb2.pitch, Pitch::E);
    EXPECT_EQ(eb2.accidental, Accidental::Flat);
    EXPECT_EQ(eb2.octave, 2);
}

TEST(Scanner, NoteLongestMatchBeatsPitchClass) {
    // `A#4` must lex as a single NOTE, not PITCH_CLASS + INT.
    const auto tokens = scan_kinds("A#4");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], S::S_NOTE_LIT);
    const auto as4 = scan_as<Note>("A#4");
    EXPECT_EQ(as4.pitch, Pitch::A);
    EXPECT_EQ(as4.accidental, Accidental::Sharp);
    EXPECT_EQ(as4.octave, 4);
}

// ===========================================================================
// Signature statement tokenization
// ===========================================================================

TEST(Scanner, SignatureStatement) {
    // "signature 4/4;" lexes as SIGNATURE INT '/' INT ';'
    const auto tokens = scan_kinds("signature 4/4;");
    ASSERT_EQ(tokens.size(), 5u);
    EXPECT_EQ(tokens[0], S::S_SIGNATURE);
    EXPECT_EQ(tokens[1], S::S_INT_LIT);
    EXPECT_EQ(tokens[2], S::S_SLASH);
    EXPECT_EQ(tokens[3], S::S_INT_LIT);
    EXPECT_EQ(tokens[4], S::S_SEMICOLON);
}

// ===========================================================================
// Identifiers
// ===========================================================================

TEST(Scanner, Identifier) {
    EXPECT_EQ(scan_kinds("intro_melody")[0], S::S_IDENT);
    EXPECT_EQ(scan_as<std::string>("intro_melody"), "intro_melody");
    EXPECT_EQ(scan_as<std::string>("_private"), "_private");
    EXPECT_EQ(scan_as<std::string>("var123"), "var123");
    EXPECT_EQ(scan_as<std::string>("myTrack"), "myTrack");
    EXPECT_EQ(scan_as<std::string>("GLOBAL_VAR"), "GLOBAL_VAR");
}

// ===========================================================================
// Operators
// ===========================================================================

TEST(Scanner, ArithmeticOperators) {
    const auto tokens = scan_kinds("+ - * / %");
    const std::vector expected = {S::S_PLUS, S::S_MINUS, S::S_STAR, S::S_SLASH, S::S_PERCENT};
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]);
}

TEST(Scanner, ComparisonOperators) {
    const auto tokens = scan_kinds("== != < > <= >=");
    const std::vector expected = {S::S_EQEQ, S::S_NEQ, S::S_LT, S::S_GT, S::S_LEQ, S::S_GEQ};
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]);
}

TEST(Scanner, LogicalOperators) {
    const auto tokens = scan_kinds("&& || !");
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], S::S_AND);
    EXPECT_EQ(tokens[1], S::S_OR);
    EXPECT_EQ(tokens[2], S::S_NOT);
}

TEST(Scanner, AssignmentVsEquality) {
    const auto tokens = scan_kinds("= ==");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], S::S_EQ);
    EXPECT_EQ(tokens[1], S::S_EQEQ);
}

// ===========================================================================
// Punctuation
// ===========================================================================

TEST(Scanner, Punctuation) {
    const auto tokens = scan_kinds("; , : ( ) [ ] { }");
    const std::vector expected = {S::S_SEMICOLON,
                                  S::S_COMMA,
                                  S::S_COLON,
                                  S::S_LPAREN,
                                  S::S_RPAREN,
                                  S::S_LBRACKET,
                                  S::S_RBRACKET,
                                  S::S_LBRACE,
                                  S::S_RBRACE};
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]);
}

// ===========================================================================
// Comments / whitespace
// ===========================================================================

TEST(Scanner, LineCommentIgnored) { EXPECT_TRUE(scan_kinds("// this is a comment").empty()); }

TEST(Scanner, BlockCommentIgnored) { EXPECT_TRUE(scan_kinds("/* block comment */").empty()); }

TEST(Scanner, BlockCommentMultiline) {
    const auto tokens = scan_kinds("/* line1\nline2\nline3 */tempo");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], S::S_TEMPO);
}

TEST(Scanner, UnterminatedBlockCommentThrows) { EXPECT_THROW(scan_kinds("/* never closed"), std::runtime_error); }

TEST(Scanner, WhitespaceIgnored) {
    const auto tokens = scan_kinds("  \t  tempo  \t  ");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0], S::S_TEMPO);
}

// ===========================================================================
// Location tracking
// ===========================================================================

TEST(Scanner, ColumnTracking) {
    const auto tokens = scan_with_loc("tempo play");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].loc.begin.column, 1);
    EXPECT_EQ(tokens[0].loc.end.column, 6);
    EXPECT_EQ(tokens[1].loc.begin.column, 7);
}

TEST(Scanner, LineTracking) {
    const auto tokens = scan_with_loc("tempo\nplay");
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].loc.begin.line, 1);
    EXPECT_EQ(tokens[1].loc.begin.line, 2);
}

// ===========================================================================
// Error cases
// ===========================================================================

TEST(Scanner, IllegalCharacterThrows) { EXPECT_THROW(scan_kinds("@"), std::runtime_error); }

TEST(Scanner, IllegalCharacterLocationReported) {
    try {
        scan_kinds("tempo @");
        FAIL() << "expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find('7'), std::string::npos);
    }
}

// ===========================================================================
// Realistic sequences
// ===========================================================================

TEST(Scanner, TempoStatement) {
    const auto tokens = scan_kinds("tempo 130;");
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], S::S_TEMPO);
    EXPECT_EQ(tokens[1], S::S_INT_LIT);
    EXPECT_EQ(tokens[2], S::S_SEMICOLON);
}

TEST(Scanner, NoteList) {
    const auto tokens = scan_kinds("[E4, F#4, G4:2]");
    const std::vector expected = {S::S_LBRACKET,
                                  S::S_NOTE_LIT,
                                  S::S_COMMA,
                                  S::S_NOTE_LIT,
                                  S::S_COMMA,
                                  S::S_NOTE_LIT,
                                  S::S_COLON,
                                  S::S_INT_LIT,
                                  S::S_RBRACKET};
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]) << "at index " << i;
}

TEST(Scanner, ChordWithPerNoteDurations) {
    // "(A3:2, B2, C3:3)" — chord items can carry their own durations
    const auto tokens = scan_kinds("(A3:2, B2, C3:3)");
    const std::vector expected = {S::S_LPAREN,
                                  S::S_NOTE_LIT,
                                  S::S_COLON,
                                  S::S_INT_LIT,
                                  S::S_COMMA,
                                  S::S_NOTE_LIT,
                                  S::S_COMMA,
                                  S::S_NOTE_LIT,
                                  S::S_COLON,
                                  S::S_INT_LIT,
                                  S::S_RPAREN};
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]) << "at index " << i;
}

TEST(Scanner, TrackHeader) {
    const auto tokens = scan_kinds("track bassline using bass {");
    const std::vector expected = {S::S_TRACK, S::S_IDENT, S::S_USING, S::S_INSTRUMENT_LIT, S::S_LBRACE};
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]) << "at index " << i;
}

TEST(Scanner, LoopStatement) {
    const auto tokens = scan_kinds("loop (3) {}");
    const std::vector expected = {S::S_LOOP, S::S_LPAREN, S::S_INT_LIT, S::S_RPAREN, S::S_LBRACE, S::S_RBRACE};
    ASSERT_EQ(tokens.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) EXPECT_EQ(tokens[i], expected[i]) << "at index " << i;
}
