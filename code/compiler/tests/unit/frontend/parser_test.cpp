#include "parser.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "dsl/ast/program.hpp"
#include "dsl/errors/syntax_error.hpp"
#include "dsl/location.hpp"
#include "dsl/music/key_mode.hpp"
#include "dsl/music/pitch.hpp"

// -- Flex interface --------------------------------------------------------
struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void scanner_reset();

// -- Aliases ---------------------------------------------------------------
namespace ast = dsl::ast;

using dsl::Location;
using dsl::errors::SyntaxError;
using dsl::frontend::Parser;
using dsl::music::Accidental;
using dsl::music::DrumNote;
using dsl::music::Instrument;
using dsl::music::Pitch;

// -- Test helpers ----------------------------------------------------------
namespace {

struct ParseGuard {
    YY_BUFFER_STATE buf;
    explicit ParseGuard(const std::string& src) {
        scanner_reset();
        buf = yy_scan_string(src.c_str());
    }
    ~ParseGuard() {
        yy_delete_buffer(buf);
        scanner_reset();
    }
    ParseGuard(const ParseGuard&) = delete;
    ParseGuard& operator=(const ParseGuard&) = delete;
};

std::unique_ptr<ast::Program> parse(const std::string& src) {
    ParseGuard guard(src);
    auto program = std::make_unique<ast::Program>();
    Location loc;
    Parser parser{loc, *program};
    const int rc = parser.parse();
    return rc == 0 ? std::move(program) : nullptr;
}

std::unique_ptr<ast::Program> parse_ok(const std::string& src) {
    auto program = parse(src);
    [&] { ASSERT_NE(program, nullptr) << "failed to parse:\n" << src; }();
    return program;
}

template <typename T>
const T& as(const ast::ExpressionKind& k) {
    return std::get<T>(k);
}

template <typename T>
const T& as_stmt(const ast::StatementPtr& sp) {
    const auto* casted = dynamic_cast<const T*>(sp.get());
    EXPECT_NE(casted, nullptr) << "statement not of expected type";
    return *casted;
}

// Global let/pattern extractors — globals are a variant<StatementPtr, PatternDefinition>.
template <typename T>
const T& global_stmt(const ast::Program& p, size_t i) {
    return as_stmt<T>(std::get<ast::StatementPtr>(p.globals[i]));
}

const ast::PatternDefinition& global_pattern(const ast::Program& p, size_t i) {
    return std::get<ast::PatternDefinition>(p.globals[i]);
}

template <typename T>
const T& track_stmt(const ast::TrackDeclaration& t, const size_t i) {
    return as_stmt<T>(std::get<ast::StatementPtr>(t.body[i]));
}

const ast::PatternDefinition& track_pattern(const ast::TrackDeclaration& t, const size_t i) {
    return std::get<ast::PatternDefinition>(t.body[i]);
}

}  // namespace

// ===========================================================================
// Empty programs
// ===========================================================================

TEST(Parser, EmptyProgram) {
    const auto p = parse_ok("");
    EXPECT_FALSE(p->header.tempo.has_value());
    EXPECT_TRUE(p->globals.empty());
    EXPECT_TRUE(p->tracks.empty());
}

TEST(Parser, OnlyWhitespaceAndComments) {
    const auto p = parse_ok("// just a comment\n  /* block */\n");
    EXPECT_TRUE(p->globals.empty());
    EXPECT_TRUE(p->tracks.empty());
}

// ===========================================================================
// Header declarations (tempo / signature / key)
// ===========================================================================

TEST(Parser, TempoDeclaration) {
    const auto p = parse_ok("tempo 130;");
    ASSERT_TRUE(p->header.tempo.has_value());
    EXPECT_EQ(p->header.tempo->bpm, 130);
}

TEST(Parser, TempoRequiresSemicolon) { EXPECT_THROW(parse("tempo 120"), SyntaxError); }

TEST(Parser, TempoRequiresInteger) { EXPECT_THROW(parse("tempo 120.0;"), SyntaxError); }

TEST(Parser, SignatureDeclaration) {
    const auto p = parse_ok("signature 4/4;");
    ASSERT_TRUE(p->header.signature.has_value());
    EXPECT_EQ(p->header.signature->beats, 4);
    EXPECT_EQ(p->header.signature->unit, 4);
}

TEST(Parser, SignatureNonStandard) {
    const auto p = parse_ok("signature 7/8;");
    EXPECT_EQ(p->header.signature->beats, 7);
    EXPECT_EQ(p->header.signature->unit, 8);
}

TEST(Parser, KeyDeclarationMajor) {
    const auto p = parse_ok("key D major;");
    ASSERT_TRUE(p->header.key.has_value());
    EXPECT_EQ(p->header.key->pitch.pitch, Pitch::D);
    EXPECT_EQ(p->header.key->pitch.accidental, Accidental::Natural);
    EXPECT_EQ(p->header.key->mode, dsl::music::KeyMode::Major);
}

TEST(Parser, KeyDeclarationMinor) {
    const auto p = parse_ok("key D minor;");
    ASSERT_TRUE(p->header.key.has_value());
    EXPECT_EQ(p->header.key->pitch.pitch, Pitch::D);
    EXPECT_EQ(p->header.key->pitch.accidental, Accidental::Natural);
    EXPECT_EQ(p->header.key->mode, dsl::music::KeyMode::Minor);
}

TEST(Parser, KeyDeclarationSharpMajor) {
    const auto p = parse_ok("key D# major;");
    EXPECT_EQ(p->header.key->pitch.pitch, Pitch::D);
    EXPECT_EQ(p->header.key->pitch.accidental, Accidental::Sharp);
    EXPECT_EQ(p->header.key->mode, dsl::music::KeyMode::Major);
}

TEST(Parser, KeyDeclarationFlatMinor) {
    const auto p = parse_ok("key Bb minor;");
    EXPECT_EQ(p->header.key->pitch.pitch, Pitch::B);
    EXPECT_EQ(p->header.key->pitch.accidental, Accidental::Flat);
    EXPECT_EQ(p->header.key->mode, dsl::music::KeyMode::Minor);
}

TEST(Parser, KeyRejectsOctave) {
    // `A4` would lex as NOTE_LIT, not PITCH_CLASS — syntax error.
    EXPECT_THROW(parse("key A4 major;"), SyntaxError);
}

TEST(Parser, KeyRejectsMissingMode) {
    // Mode is required — `key D#;` without major/minor is a syntax error.
    EXPECT_THROW(parse("key D#;"), SyntaxError);
}

TEST(Parser, AllHeadersTogether) {
    const auto p = parse_ok("tempo 130; signature 4/4; key D# major;");
    EXPECT_EQ(p->header.tempo->bpm, 130);
    EXPECT_EQ(p->header.signature->beats, 4);
    EXPECT_EQ(p->header.key->pitch.pitch, Pitch::D);
    EXPECT_EQ(p->header.key->pitch.accidental, Accidental::Sharp);
    EXPECT_EQ(p->header.key->mode, dsl::music::KeyMode::Major);
}

TEST(Parser, HeadersInAnyOrder) {
    const auto p = parse_ok("key C minor; tempo 100; signature 3/4;");
    EXPECT_EQ(p->header.tempo->bpm, 100);
    EXPECT_EQ(p->header.signature->beats, 3);
    EXPECT_EQ(p->header.key->pitch.pitch, Pitch::C);
    EXPECT_EQ(p->header.key->pitch.accidental, Accidental::Natural);
    EXPECT_EQ(p->header.key->mode, dsl::music::KeyMode::Minor);
}

TEST(Parser, DuplicateTempoRejected) { EXPECT_THROW(parse("tempo 120; tempo 130;"), SyntaxError); }
TEST(Parser, DuplicateSignatureRejected) { EXPECT_THROW(parse("signature 4/4; signature 3/4;"), SyntaxError); }
TEST(Parser, DuplicateKeyRejected) { EXPECT_THROW(parse("key C major; key D minor;"), SyntaxError); }

TEST(Parser, HeaderAfterTrackRejected) { EXPECT_THROW(parse("track {} tempo 120;"), SyntaxError); }

TEST(Parser, HeaderAfterGlobalLetRejected) { EXPECT_THROW(parse("let x = 1; tempo 120;"), SyntaxError); }

// ===========================================================================
// Global `let` and pattern definitions
// ===========================================================================

TEST(Parser, GlobalLet) {
    const auto p = parse_ok("let x = 42;");
    ASSERT_EQ(p->globals.size(), 1u);
    const auto& let = global_stmt<ast::LetStatement>(*p, 0);
    EXPECT_EQ(let.name, "x");
    EXPECT_EQ(as<ast::IntLiteral>(let.value->kind).value, 42);
}

TEST(Parser, GlobalPatternEmpty) {
    const auto p = parse_ok("pattern verse() {}");
    ASSERT_EQ(p->globals.size(), 1u);
    const auto& pat = global_pattern(*p, 0);
    EXPECT_EQ(pat.name, "verse");
    EXPECT_TRUE(pat.params.empty());
    EXPECT_TRUE(pat.body.empty());
}

TEST(Parser, GlobalPatternWithParams) {
    const auto p = parse_ok("pattern verse(a, b, c) {}");
    const auto& pat = global_pattern(*p, 0);
    ASSERT_EQ(pat.params.size(), 3u);
    EXPECT_EQ(pat.params[0], "a");
    EXPECT_EQ(pat.params[2], "c");
}

TEST(Parser, GlobalAssignmentRejected) {
    // Assignment is not allowed at global scope — only `let`.
    EXPECT_THROW(parse("x = 7;"), SyntaxError);
}

TEST(Parser, GlobalPlayRejected) { EXPECT_THROW(parse("play A4;"), SyntaxError); }

TEST(Parser, GlobalForRejected) { EXPECT_THROW(parse("for (;;) {}"), SyntaxError); }

TEST(Parser, GlobalLoopRejected) { EXPECT_THROW(parse("loop (3) {}"), SyntaxError); }

TEST(Parser, GlobalIfRejected) { EXPECT_THROW(parse("if (true) {}"), SyntaxError); }

// ===========================================================================
// Tracks
// ===========================================================================

TEST(Parser, AnonymousTrackEmpty) {
    const auto p = parse_ok("track {}");
    ASSERT_EQ(p->tracks.size(), 1u);
    EXPECT_FALSE(p->tracks[0].name.has_value());
    EXPECT_FALSE(p->tracks[0].instrument.has_value());
    EXPECT_TRUE(p->tracks[0].body.empty());
}

TEST(Parser, NamedTrack) {
    const auto p = parse_ok("track bassline {}");
    ASSERT_EQ(p->tracks.size(), 1u);
    ASSERT_TRUE(p->tracks[0].name.has_value());
    EXPECT_EQ(*p->tracks[0].name, "bassline");
    EXPECT_FALSE(p->tracks[0].instrument.has_value());
}

TEST(Parser, TrackUsingInstrument) {
    const auto p = parse_ok("track using drums {}");
    ASSERT_TRUE(p->tracks[0].instrument.has_value());
    EXPECT_EQ(*p->tracks[0].instrument, Instrument::Drums);
    EXPECT_FALSE(p->tracks[0].name.has_value());
}

TEST(Parser, TrackNamedWithInstrument) {
    const auto p = parse_ok("track bassline using bass {}");
    EXPECT_EQ(*p->tracks[0].name, "bassline");
    EXPECT_EQ(*p->tracks[0].instrument, Instrument::Bass);
}

TEST(Parser, TrackWithMixedBody) {
    const auto p = parse_ok(R"(
        track {
            let x = 1;
            pattern p() { play A4; }
            play A4;
            loop (2) { play B4; }
        }
    )");
    const auto& t = p->tracks[0];
    ASSERT_EQ(t.body.size(), 4u);
    EXPECT_NO_THROW(track_stmt<ast::LetStatement>(t, 0));
    EXPECT_NO_THROW(track_pattern(t, 1));
    EXPECT_NO_THROW(track_stmt<ast::PlayStatement>(t, 2));
    EXPECT_NO_THROW(track_stmt<ast::LoopStatement>(t, 3));
}

TEST(Parser, MultipleTracks) {
    const auto p = parse_ok("track {} track bassline using bass {} track using drums {}");
    ASSERT_EQ(p->tracks.size(), 3u);
}

TEST(Parser, TrackRequiresBraces) { EXPECT_THROW(parse("track bassline;"), SyntaxError); }

// ===========================================================================
// Pattern definitions (bodies cannot nest patterns)
// ===========================================================================

TEST(Parser, NestedPatternDefinitionRejected) {
    EXPECT_THROW(parse("pattern outer() { pattern inner() {} }"), SyntaxError);
}

TEST(Parser, PatternCanPlayAnotherPattern) {
    const auto p = parse_ok(R"(
        pattern verse() {
            play chorus();
        }
    )");
    const auto& pat = global_pattern(*p, 0);
    ASSERT_EQ(pat.body.size(), 1u);
    const auto& play = as_stmt<ast::PlayStatement>(pat.body[0]);
    const auto& src = std::get<ast::ExpressionPtr>(play.target.source);
    const auto& [callee, args] = as<ast::Call>(src->kind);
    EXPECT_EQ(callee, "chorus");
}

TEST(Parser, PatternInsideTrack) {
    const auto p = parse_ok(R"(
        track {
            pattern verse() { play A4; }
        }
    )");
    const auto& t = p->tracks[0];
    ASSERT_EQ(t.body.size(), 1u);
    const auto& pat = track_pattern(t, 0);
    EXPECT_EQ(pat.name, "verse");
}

// ===========================================================================
// Play statement — sources
// ===========================================================================

namespace {
const ast::PlayStatement& first_play_in_track(const ast::Program& p) {
    return track_stmt<ast::PlayStatement>(p.tracks[0], 0);
}
}  // namespace

TEST(Parser, PlayNoteLiteral) {
    const auto p = parse_ok("track { play A4; }");
    const auto& play = first_play_in_track(*p);
    const auto& src = std::get<ast::ExpressionPtr>(play.target.source);
    const auto& note = as<ast::NoteLiteral>(src->kind).value;
    EXPECT_EQ(note.pitch, Pitch::A);
    EXPECT_EQ(note.accidental, Accidental::Natural);
    EXPECT_EQ(note.octave, 4);
    EXPECT_EQ(play.target.duration, nullptr);
    EXPECT_EQ(play.target.from_offset, nullptr);
}

TEST(Parser, PlayRest) {
    const auto p = parse_ok("track { play rest; }");
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(*p).target.source);
    EXPECT_TRUE(std::holds_alternative<ast::RestLiteral>(src->kind));
}

TEST(Parser, PlayDrumNote) {
    const auto p = parse_ok("track using drums { play kick; }");
    const auto& src = first_play_in_track(*p).target.source;
    ASSERT_TRUE(std::holds_alternative<DrumNote>(src));
    EXPECT_EQ(std::get<DrumNote>(src), DrumNote::Kick);
}

TEST(Parser, PlayChord) {
    const auto p = parse_ok("track { play (C4, E4, G4); }");
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(*p).target.source);
    const auto& [notes] = as<ast::Chord>(src->kind);
    EXPECT_EQ(notes.size(), 3u);
}

TEST(Parser, PlaySequence) {
    const auto p = parse_ok("track { play [A4, B4, C4]; }");
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(*p).target.source);
    const auto& [items] = as<ast::Sequence>(src->kind);
    EXPECT_EQ(items.size(), 3u);
}

TEST(Parser, PlayIdentifier) {
    const auto p = parse_ok("track { play my_pattern; }");
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(*p).target.source);
    EXPECT_EQ(as<ast::Identifier>(src->kind).name, "my_pattern");
}

TEST(Parser, PlayCallNoArgs) {
    const auto p = parse_ok("track { play verse(); }");
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(*p).target.source);
    const auto& [callee, args] = as<ast::Call>(src->kind);
    EXPECT_EQ(callee, "verse");
    EXPECT_TRUE(args.empty());
}

TEST(Parser, PlayCallWithArgs) {
    const auto p = parse_ok("track { play verse(1, x, [A4]); }");
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(*p).target.source);
    const auto& [callee, args] = as<ast::Call>(src->kind);
    ASSERT_EQ(args.size(), 3u);
    EXPECT_EQ(as<ast::IntLiteral>(args[0]->kind).value, 1);
    EXPECT_EQ(as<ast::Identifier>(args[1]->kind).name, "x");
    EXPECT_TRUE(std::holds_alternative<ast::Sequence>(args[2]->kind));
}

// ===========================================================================
// Play statement — modifiers
// ===========================================================================

TEST(Parser, PlayWithDuration) {
    const auto p = parse_ok("track { play A4:4; }");
    const auto& play = first_play_in_track(*p);
    ASSERT_NE(play.target.duration, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(play.target.duration->kind).value, 4);
}

TEST(Parser, PlayChordWithDuration) {
    const auto p = parse_ok("track { play (E3, G3, B3):4; }");
    const auto& play = first_play_in_track(*p);
    ASSERT_NE(play.target.duration, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(play.target.duration->kind).value, 4);
}

TEST(Parser, PlayChordWithPerNoteDurations) {
    const auto p = parse_ok("track { play (A3:2, B2, C3:3); }");
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(*p).target.source);
    const auto& [notes] = as<ast::Chord>(src->kind);
    ASSERT_EQ(notes.size(), 3u);
    ASSERT_NE(notes[0].duration, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(notes[0].duration->kind).value, 2);
    EXPECT_EQ(notes[1].duration, nullptr);
    ASSERT_NE(notes[2].duration, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(notes[2].duration->kind).value, 3);
}

TEST(Parser, PlayRestWithFloatDuration) {
    const auto p = parse_ok("track { play rest:0.5; }");
    const auto& play = first_play_in_track(*p);
    ASSERT_NE(play.target.duration, nullptr);
    EXPECT_DOUBLE_EQ(as<ast::FloatLiteral>(play.target.duration->kind).value, 0.5);
}

TEST(Parser, PlayWithFromOffset) {
    const auto p = parse_ok("track { play A4 from 16; }");
    const auto& play = first_play_in_track(*p);
    ASSERT_NE(play.target.from_offset, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(play.target.from_offset->kind).value, 16);
}

TEST(Parser, PlayCallFromOffset) {
    const auto p = parse_ok("track { play verse(x) from 1; }");
    const auto& play = first_play_in_track(*p);
    ASSERT_NE(play.target.from_offset, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(play.target.from_offset->kind).value, 1);
}

TEST(Parser, PlayNoteWithDurationAndFrom) {
    const auto p = parse_ok("track { play A4:4 from 1; }");
    const auto& play = first_play_in_track(*p);
    ASSERT_NE(play.target.duration, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(play.target.duration->kind).value, 4);
    ASSERT_NE(play.target.from_offset, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(play.target.from_offset->kind).value, 1);
}

TEST(Parser, PlayUsingInstrumentRejected) {
    // `using` attaches only to tracks, not play statements.
    EXPECT_THROW(parse("track { play A4 using piano; }"), SyntaxError);
}

TEST(Parser, PlaySequenceCannotHaveDuration) { EXPECT_THROW(parse("track { play [A4, B4]:4; }"), SyntaxError); }

TEST(Parser, PlayDrumNoteCannotHaveDuration) { EXPECT_THROW(parse("track { play kick:4; }"), SyntaxError); }

// ===========================================================================
// Sequences
// ===========================================================================

namespace {
// Convenience: parse inside a track and return the first play's sequence.
const ast::Sequence& first_sequence(const ast::Program& p) {
    const auto& src = std::get<ast::ExpressionPtr>(first_play_in_track(p).target.source);
    return as<ast::Sequence>(src->kind);
}
}  // namespace

TEST(Parser, SequenceItemsWithDurationsAndRest) {
    const auto p = parse_ok("track { play [A4, B4:2, rest:0.5]; }");
    const auto& [items] = first_sequence(*p);
    ASSERT_EQ(items.size(), 3u);
    EXPECT_EQ(items[0].duration, nullptr);
    ASSERT_NE(items[1].duration, nullptr);
    EXPECT_EQ(as<ast::IntLiteral>(items[1].duration->kind).value, 2);
    ASSERT_NE(items[2].duration, nullptr);
    EXPECT_DOUBLE_EQ(as<ast::FloatLiteral>(items[2].duration->kind).value, 0.5);
    EXPECT_TRUE(std::holds_alternative<ast::RestLiteral>(items[2].value->kind));
}

TEST(Parser, SequenceSingleton) {
    const auto p = parse_ok("track { play [A4:4]; }");
    EXPECT_EQ(first_sequence(*p).items.size(), 1u);
}

TEST(Parser, EmptySequenceRejected) { EXPECT_THROW(parse("track { play []; }"), SyntaxError); }

// ===========================================================================
// Chords
// ===========================================================================

TEST(Parser, ChordRequiresAtLeastTwoElements) {
    // "(X)" in expression position is a parenthesised expression.
    const auto p = parse_ok("let x = (A4);");
    const auto& let = global_stmt<ast::LetStatement>(*p, 0);
    EXPECT_TRUE(std::holds_alternative<ast::ParenthesisedExpression>(let.value->kind));
}

TEST(Parser, ChordTwoElements) {
    const auto p = parse_ok("let x = (A4, C5);");
    const auto& let = global_stmt<ast::LetStatement>(*p, 0);
    const auto& [notes] = as<ast::Chord>(let.value->kind);
    EXPECT_EQ(notes.size(), 2u);
}

TEST(Parser, ChordRestRejected) {
    // `rest` is not a general expression; it cannot appear inside a chord.
    EXPECT_THROW(parse("let x = (A4, rest);"), SyntaxError);
}

// ===========================================================================
// For / loop / if statements (inside patterns)
// ===========================================================================

TEST(Parser, ForLoopBasic) {
    const auto p = parse_ok(R"(
        pattern p() {
            for (let i = 0; i < 4; i = i + 1) {
                play A4;
            }
        }
    )");
    const auto& pat = global_pattern(*p, 0);
    const auto& f = as_stmt<ast::ForStatement>(pat.body[0]);
    EXPECT_NE(dynamic_cast<const ast::LetStatement*>(f.init.get()), nullptr);
    ASSERT_NE(f.cond, nullptr);
    EXPECT_TRUE(std::holds_alternative<ast::BinaryExpression>(f.cond->kind));
    EXPECT_NE(dynamic_cast<const ast::AssignStatement*>(f.step.get()), nullptr);
    EXPECT_EQ(f.body.size(), 1u);
}

TEST(Parser, ForLoopAllOmitted) {
    const auto p = parse_ok("pattern p() { for (;;) { play A4; } }");
    const auto& f = as_stmt<ast::ForStatement>(global_pattern(*p, 0).body[0]);
    EXPECT_EQ(f.init, nullptr);
    EXPECT_EQ(f.cond, nullptr);
    EXPECT_EQ(f.step, nullptr);
}

TEST(Parser, ForInitMustNotBePlay) {
    EXPECT_THROW(parse("pattern p() { for (play A4; true; i = i + 1) {} }"), SyntaxError);
}

TEST(Parser, LoopBasic) {
    const auto p = parse_ok("pattern p() { loop (3) { play A4; } }");
    const auto& l = as_stmt<ast::LoopStatement>(global_pattern(*p, 0).body[0]);
    EXPECT_EQ(as<ast::IntLiteral>(l.count->kind).value, 3);
    EXPECT_EQ(l.body.size(), 1u);
}

TEST(Parser, LoopAcceptsExpression) {
    const auto p = parse_ok("pattern p() { loop (n + 1) { play A4; } }");
    const auto& l = as_stmt<ast::LoopStatement>(global_pattern(*p, 0).body[0]);
    EXPECT_TRUE(std::holds_alternative<ast::BinaryExpression>(l.count->kind));
}

TEST(Parser, LoopRequiresParens) { EXPECT_THROW(parse("pattern p() { loop 3 { play A4; } }"), SyntaxError); }

TEST(Parser, IfNoElse) {
    const auto p = parse_ok("pattern p() { if (true) { play A4; } }");
    const auto& i = as_stmt<ast::IfStatement>(global_pattern(*p, 0).body[0]);
    EXPECT_TRUE(std::holds_alternative<ast::BoolLiteral>(i.cond->kind));
    EXPECT_EQ(i.then_branch.size(), 1u);
    EXPECT_FALSE(i.else_branch.has_value());
}

TEST(Parser, IfWithElse) {
    const auto p = parse_ok("pattern p() { if (i < 2) { play A4; } else { play B4; } }");
    const auto& i = as_stmt<ast::IfStatement>(global_pattern(*p, 0).body[0]);
    ASSERT_TRUE(i.else_branch.has_value());
    EXPECT_EQ(i.else_branch->size(), 1u);
}

// ===========================================================================
// Expressions — literals & precedence
// ===========================================================================

namespace {
const ast::Expression& first_global_let_value(const ast::Program& p) {
    return *global_stmt<ast::LetStatement>(p, 0).value;
}
}  // namespace

TEST(Parser, IntLiteralExpr) {
    const auto p = parse_ok("let x = 42;");
    EXPECT_EQ(as<ast::IntLiteral>(first_global_let_value(*p).kind).value, 42);
}

TEST(Parser, FloatLiteralExpr) {
    const auto p = parse_ok("let x = 3.14;");
    EXPECT_DOUBLE_EQ(as<ast::FloatLiteral>(first_global_let_value(*p).kind).value, 3.14);
}

TEST(Parser, BoolLiteralExpr) {
    const auto p = parse_ok("let x = true;");
    EXPECT_TRUE(as<ast::BoolLiteral>(first_global_let_value(*p).kind).value);
}

TEST(Parser, IdentifierExpr) {
    const auto p = parse_ok("let x = y;");
    EXPECT_EQ(as<ast::Identifier>(first_global_let_value(*p).kind).name, "y");
}

TEST(Parser, MulBindsTighterThanAdd) {
    const auto p = parse_ok("let x = 1 + 2 * 3;");
    const auto& top = as<ast::BinaryExpression>(first_global_let_value(*p).kind);
    EXPECT_EQ(top.op, ast::BinaryOperator::Add);
    EXPECT_EQ(as<ast::BinaryExpression>(top.rhs->kind).op, ast::BinaryOperator::Multiply);
}

TEST(Parser, AddLeftAssociative) {
    const auto p = parse_ok("let x = 1 - 2 - 3;");
    const auto& top = as<ast::BinaryExpression>(first_global_let_value(*p).kind);
    EXPECT_EQ(top.op, ast::BinaryOperator::Subtract);
    EXPECT_EQ(as<ast::BinaryExpression>(top.lhs->kind).op, ast::BinaryOperator::Subtract);
}

TEST(Parser, ParensOverridePrecedence) {
    const auto p = parse_ok("let x = 3 * (2 + 1);");
    const auto& top = as<ast::BinaryExpression>(first_global_let_value(*p).kind);
    EXPECT_EQ(top.op, ast::BinaryOperator::Multiply);
    const auto& [inner] = as<ast::ParenthesisedExpression>(top.rhs->kind);
    EXPECT_EQ(as<ast::BinaryExpression>(inner->kind).op, ast::BinaryOperator::Add);
}

TEST(Parser, ComparisonBindsLooserThanAdd) {
    const auto p = parse_ok("let x = 1 + 2 < 4;");
    EXPECT_EQ(as<ast::BinaryExpression>(first_global_let_value(*p).kind).op, ast::BinaryOperator::Less);
}

TEST(Parser, AndBindsTighterThanOr) {
    const auto p = parse_ok("let x = a || b && c;");
    const auto& top = as<ast::BinaryExpression>(first_global_let_value(*p).kind);
    EXPECT_EQ(top.op, ast::BinaryOperator::Or);
    EXPECT_EQ(as<ast::BinaryExpression>(top.rhs->kind).op, ast::BinaryOperator::And);
}

TEST(Parser, ModuloBindsLikeMultiply) {
    const auto p = parse_ok("let x = i % 4 == 0;");
    const auto& top = as<ast::BinaryExpression>(first_global_let_value(*p).kind);
    EXPECT_EQ(top.op, ast::BinaryOperator::Equals);
    EXPECT_EQ(as<ast::BinaryExpression>(top.lhs->kind).op, ast::BinaryOperator::Modulo);
}

TEST(Parser, UnaryNegation) {
    const auto p = parse_ok("let x = -5;");
    const auto& [op, operand] = as<ast::UnaryExpression>(first_global_let_value(*p).kind);
    EXPECT_EQ(op, ast::UnaryOperator::Negative);
}

TEST(Parser, UnaryNot) {
    const auto p = parse_ok("let x = !true;");
    const auto& [op, operand] = as<ast::UnaryExpression>(first_global_let_value(*p).kind);
    EXPECT_EQ(op, ast::UnaryOperator::Not);
}

// Pattern calls are only legal in play-source position.
TEST(Parser, CallNotAllowedInExpression) { EXPECT_THROW(parse("let x = f(1, 2);"), SyntaxError); }
TEST(Parser, CallNotAllowedInExpressionNoArgs) { EXPECT_THROW(parse("let x = f();"), SyntaxError); }

// `rest` is not a general expression.
TEST(Parser, RestNotAllowedInExpression) { EXPECT_THROW(parse("let x = rest;"), SyntaxError); }

// ===========================================================================
// Location tracking
// ===========================================================================

TEST(Parser, TempoLocationReported) {
    const auto p = parse_ok("tempo 120;");
    EXPECT_EQ(p->header.tempo->loc.begin.line, 1);
    EXPECT_EQ(p->header.tempo->loc.begin.column, 1);
}

TEST(Parser, TrackLocationReported) {
    const auto p = parse_ok("\n\ntrack {}");
    EXPECT_EQ(p->tracks[0].loc.begin.line, 3);
}

// ===========================================================================
// Syntax error reporting
// ===========================================================================

TEST(Parser, SyntaxErrorThrows) { EXPECT_THROW(parse("tempo ;"), SyntaxError); }

TEST(Parser, SyntaxErrorMessageIsDetailed) {
    try {
        parse("tempo ;");
        FAIL() << "expected SyntaxError";
    } catch (const SyntaxError& e) {
        EXPECT_NE(std::string(e.what()).find("expected"), std::string::npos);
    }
}

TEST(Parser, UnclosedBraceRejected) { EXPECT_THROW(parse("pattern p() { play A4;"), SyntaxError); }

TEST(Parser, StrayTokenRejected) { EXPECT_THROW(parse(";"), SyntaxError); }

// ===========================================================================
// Realistic program — mirrors tests/data/example.dsl
// ===========================================================================

TEST(Parser, RealisticProgram) {
    static constexpr auto kSrc = R"(
        tempo 130;
        signature 4/4;
        key D# major;

        let GLOBAL_VAR = 10;
        let GLOBAL_SEQ = [A4, rest, B4:2, C4:3];
        let GLOBAL_CHORD = (A3:2, B2, C3:3);

        pattern global_pattern() {}

        track {
            pattern intro_melody(my_sequence) {
                play my_sequence;
                play [A4, B4, A4, G4];
            }
            let my_seq = [A2, B2:2, rest:3, C#3:1];
            play (A3, C#3);
            play intro_melody(my_seq) from 1;
        }

        track bassline using bass {
            for (let i = 0; i < 4; i = i + 1) {
                if (i < 2) { play [E2, E2, B2, B2]; }
                else { play [A2, A2, G2, G2]; }
            }
            loop (3) {}
            play (E2, B2):16;
        }

        track bassline using bass {}

        track using drums {
            pattern rock_beat() {
                for (let i = 0; i < 16; i = i + 1) {
                    play hihat;
                    let my_var = 123;
                    my_var = 2345;
                }
            }
            play rock_beat() from 17;
        }
    )";

    const auto p = parse_ok(kSrc);
    EXPECT_EQ(p->header.tempo->bpm, 130);
    EXPECT_EQ(p->header.signature->beats, 4);
    EXPECT_EQ(p->header.signature->unit, 4);
    EXPECT_EQ(p->header.key->pitch.pitch, Pitch::D);
    EXPECT_EQ(p->header.key->pitch.accidental, Accidental::Sharp);
    EXPECT_EQ(p->header.key->mode, dsl::music::KeyMode::Major);

    // 3 lets + 1 pattern = 4 globals
    EXPECT_EQ(p->globals.size(), 4u);
    // 4 tracks total
    ASSERT_EQ(p->tracks.size(), 4u);

    EXPECT_FALSE(p->tracks[0].name.has_value());
    EXPECT_FALSE(p->tracks[0].instrument.has_value());
    EXPECT_EQ(*p->tracks[1].name, "bassline");
    EXPECT_EQ(*p->tracks[1].instrument, Instrument::Bass);
    EXPECT_EQ(*p->tracks[2].name, "bassline");
    EXPECT_EQ(*p->tracks[2].instrument, Instrument::Bass);
    EXPECT_FALSE(p->tracks[3].name.has_value());
    EXPECT_EQ(*p->tracks[3].instrument, Instrument::Drums);
}

// ===========================================================================
// Ternary operator
// ===========================================================================

TEST(Parser, TernaryParsesInExpression) { EXPECT_NE(parse("track { play (1==1 ? A4 : B4); }"), nullptr); }

TEST(Parser, TernaryParsesAsDuration) { EXPECT_NE(parse("track { play A4 :(1==1 ? 2 : 3); }"), nullptr); }

TEST(Parser, TernaryParsesInLet) { EXPECT_NE(parse("track { let x = 1==1 ? 2 : 3; }"), nullptr); }

// ===========================================================================
// Optional braces on control flow
// ===========================================================================

TEST(Parser, NoBraceIfParses) { EXPECT_NE(parse("track { if (1==1) play A4; }"), nullptr); }

TEST(Parser, NoBraceIfElseParses) { EXPECT_NE(parse("track { if (1==2) play A4; else play B4; }"), nullptr); }

TEST(Parser, NoBraceElseIfChainParses) {
    EXPECT_NE(parse("track { if (1==2) play A4; else if (1==1) play B4; else play C4; }"), nullptr);
}

TEST(Parser, NoBraceLoopParses) { EXPECT_NE(parse("track { loop (3) play A4; }"), nullptr); }

TEST(Parser, NoBraceForParses) { EXPECT_NE(parse("track { for (let i = 0; i < 3; i = i + 1) play A4; }"), nullptr); }

TEST(Parser, LetAsNoBraceBodyIsRejected) { EXPECT_THROW(parse("track { if (1==1) let x = 3; }"), SyntaxError); }

// ===========================================================================
// Voice parallelism
// ===========================================================================

// -- Acceptance --

TEST(Parser, VoiceParsesInTrack) { EXPECT_NE(parse("track { voice { play A4; } }"), nullptr); }

TEST(Parser, VoiceFromLiteralParses) { EXPECT_NE(parse("track { voice from 4 { play A4; } }"), nullptr); }

TEST(Parser, VoiceFromExprParses) { EXPECT_NE(parse("track { let n = 2; voice from n + 1 { play A4; } }"), nullptr); }

TEST(Parser, VoiceWithLocalPatternParses) {
    EXPECT_NE(parse("track { voice { pattern p() { play A4; } play p(); } }"), nullptr);
}

// -- Rejection --

TEST(Parser, VoiceNestedIsRejected) { EXPECT_THROW(parse("track { voice { voice { play A4; } } }"), SyntaxError); }

TEST(Parser, VoiceInsidePatternIsRejected) { EXPECT_THROW(parse("pattern p() { voice { play A4; } }"), SyntaxError); }

TEST(Parser, VoiceInsideLoopIsRejected) {
    EXPECT_THROW(parse("track { loop (2) { voice { play A4; } } }"), SyntaxError);
}

TEST(Parser, VoiceInsideIfIsRejected) {
    EXPECT_THROW(parse("track { if (true) { voice { play A4; } } }"), SyntaxError);
}
