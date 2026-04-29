#include "dsl/ir/lowerer.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>

#include "dsl/core/ast/program.hpp"
#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/core/music/instrument.hpp"
#include "dsl/core/music/key_mode.hpp"
#include "dsl/core/music/pitch.hpp"
#include "dsl/ir/program.hpp"
#include "parser.hpp"

// -- Flex interface --------------------------------------------------------
struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void scanner_reset();

// -- Aliases ---------------------------------------------------------------
namespace ast = dsl::ast;

using dsl::Location;
using dsl::errors::SemanticError;
using dsl::frontend::Parser;
using dsl::ir::Lowerer;
using dsl::ir::ProgramIR;
using dsl::music::Accidental;
using dsl::music::Instrument;
using dsl::music::Pitch;

// -- Helpers ---------------------------------------------------------------
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
    return parser.parse() == 0 ? std::move(program) : nullptr;
}

ProgramIR lower(const std::string& src) {
    const auto program = parse(src);
    EXPECT_NE(program, nullptr) << "parse failed for: " << src;
    return Lowerer{}.lower(*program);
}

}  // namespace

// ===========================================================================
// Default output
// ===========================================================================

TEST(Lowerer, EmptyProgram) {
    auto [tempo_bpm, time_sig_numerator, time_sig_denominator, key, tracks] = lower("");
    EXPECT_EQ(tempo_bpm, 120);
    EXPECT_EQ(time_sig_numerator, 4);
    EXPECT_EQ(time_sig_denominator, 4);
    EXPECT_FALSE(key.has_value());
    EXPECT_TRUE(tracks.empty());
}

// ===========================================================================
// Header
// ===========================================================================

TEST(Lowerer, HeaderTempo) {
    const auto ir = lower("tempo 90;");
    EXPECT_EQ(ir.tempo_bpm, 90);
}

TEST(Lowerer, HeaderSignature) {
    const auto ir = lower("signature 3/4;");
    EXPECT_EQ(ir.time_sig_numerator, 3);
    EXPECT_EQ(ir.time_sig_denominator, 4);
}

TEST(Lowerer, HeaderKey) {
    const auto ir = lower("key D# major;");
    ASSERT_TRUE(ir.key.has_value());
    EXPECT_EQ(ir.key->pitch_class.pitch, Pitch::D);
    EXPECT_EQ(ir.key->pitch_class.accidental, Accidental::Sharp);
    EXPECT_EQ(ir.key->mode, dsl::music::KeyMode::Major);
}

TEST(Lowerer, HeaderKeyMinor) {
    const auto ir = lower("key Bb minor;");
    ASSERT_TRUE(ir.key.has_value());
    EXPECT_EQ(ir.key->pitch_class.pitch, Pitch::B);
    EXPECT_EQ(ir.key->pitch_class.accidental, Accidental::Flat);
    EXPECT_EQ(ir.key->mode, dsl::music::KeyMode::Minor);
}

// ===========================================================================
// Single note
// ===========================================================================

TEST(Lowerer, SingleNotePlay) {
    const auto ir = lower("track { play A4; }");
    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
    EXPECT_EQ(ir.tracks[0].events[0].velocity, 100);
}

TEST(Lowerer, NotesMidiMapping) {
    // C4=72, D4=74, E4=76, F4=77, G4=79, A4=81, B4=83
    const auto ir = lower("track { play C4; play D4; play E4; play F4; play G4; play A4; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 7u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 72);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 74);
    EXPECT_EQ(ir.tracks[0].events[2].midi_note, 76);
    EXPECT_EQ(ir.tracks[0].events[3].midi_note, 77);
    EXPECT_EQ(ir.tracks[0].events[4].midi_note, 79);
    EXPECT_EQ(ir.tracks[0].events[5].midi_note, 81);
    EXPECT_EQ(ir.tracks[0].events[6].midi_note, 83);
}

TEST(Lowerer, SharpFlatMidi) {
    const auto ir = lower("track { play C#4; play Db4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 73);  // C#4
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 73);  // Db4 == C#4
}

// ===========================================================================
// Cursor advancement
// ===========================================================================

TEST(Lowerer, PlayAdvancesCursor) {
    const auto ir = lower("track { play A4; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
}

TEST(Lowerer, PlayExplicitDuration) {
    const auto ir = lower("track { play A4 :2; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
}

TEST(Lowerer, PlayFromOffsetDoesNotAdvanceCursor) {
    // `from` is absolute and non-advancing; B4 plays at cursor (still 0).
    const auto ir = lower("track { play A4 from 2; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    const auto& a4 = ir.tracks[0].events[0].start_beat == 2.0 ? ir.tracks[0].events[0] : ir.tracks[0].events[1];
    const auto& b4 = ir.tracks[0].events[0].start_beat == 0.0 ? ir.tracks[0].events[0] : ir.tracks[0].events[1];
    EXPECT_DOUBLE_EQ(a4.start_beat, 2.0);
    EXPECT_DOUBLE_EQ(b4.start_beat, 0.0);
}

// ===========================================================================
// Rest
// ===========================================================================

TEST(Lowerer, RestSkipsBeats) {
    const auto ir = lower("track { play rest :2; play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 2.0);
}

TEST(Lowerer, RestAdvancesCursorByOne) {
    // play rest; with no explicit duration must advance the cursor by 1 beat.
    const auto ir = lower("track { play rest; play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 1.0);
}

TEST(Lowerer, RestExplicitDurationAdvancesCursor) {
    // play rest :2; must advance cursor by 2 and emit zero events.
    const auto ir = lower("track { play rest :2; play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 1.0);
}

TEST(Lowerer, SequenceRestDefaultDuration) {
    // A rest inside a sequence with no explicit duration takes 1 beat.
    const auto ir = lower("track { play [A4, rest, B4]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    // Events are sorted by start beat.
    const auto& a4 = ir.tracks[0].events[0];
    const auto& b4 = ir.tracks[0].events[1];
    EXPECT_EQ(a4.midi_note, 81);
    EXPECT_DOUBLE_EQ(a4.start_beat, 0.0);
    EXPECT_EQ(b4.midi_note, 83);
    EXPECT_DOUBLE_EQ(b4.start_beat, 2.0);
}

TEST(Lowerer, SequenceRestExplicitDuration) {
    // A rest inside a sequence with explicit :2 duration takes 2 beats.
    const auto ir = lower("track { play [A4, rest :2, B4]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    const auto& a4 = ir.tracks[0].events[0];
    const auto& b4 = ir.tracks[0].events[1];
    EXPECT_EQ(a4.midi_note, 81);
    EXPECT_DOUBLE_EQ(a4.start_beat, 0.0);
    EXPECT_EQ(b4.midi_note, 83);
    EXPECT_DOUBLE_EQ(b4.start_beat, 3.0);
}

TEST(Lowerer, SequenceTrailingRestAdvancesCursor) {
    // A trailing rest at the end of a sequence must advance the cursor so the
    // next play statement starts after the rest, not immediately after the last note.
    const auto ir = lower("track { play [A4, rest :3]; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    double a4_start = -1.0, b4_start = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(b4_start, 4.0);  // 1 (A4) + 3 (rest)
}

TEST(Lowerer, SequenceLeadingRestAdvancesCursor) {
    // A leading rest inside a sequence must also be counted for cursor advancement.
    const auto ir = lower("track { play [rest :2, A4]; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    double a4_start = -1.0, b4_start = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 2.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);  // 2 (rest) + 1 (A4)
}

TEST(Lowerer, MultipleSequencesWithTrailingRests) {
    // Each sequence's trailing rest must accumulate correctly across multiple plays.
    // play [A4, rest:3] → 4 beats; play [B4, rest:3] → 4 beats; play C4 at beat 8
    const auto ir = lower("track { play [A4, rest :3]; play [B4, rest :3]; play C4; }");
    double a4_start = -1.0, b4_start = -1.0, c4_start = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
        if (ev.midi_note == 72) c4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(b4_start, 4.0);
    EXPECT_DOUBLE_EQ(c4_start, 8.0);
}

TEST(Lowerer, PatternBodyRestPreservesGap) {
    // A rest inside a called pattern must shift subsequent notes in that pattern.
    const auto ir = lower(
        "pattern r() { play A4; play rest :2; play B4; }\n"
        "track { play r(); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    double a4_start = -1.0, b4_start = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
}

TEST(Lowerer, PatternBodyRestAdvancesOuterCursor) {
    // After playing a pattern with internal rests the outer cursor reflects
    // the full temporal span (including the rest beats).
    const auto ir = lower(
        "pattern r() { play A4; play rest :2; play B4; }\n"
        "track { play r(); play C4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    double c4_start = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 72) c4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(c4_start, 4.0);
}

// ===========================================================================
// Sequence
// ===========================================================================

TEST(Lowerer, SequenceExpands) {
    const auto ir = lower("track { play [A4, B4]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 83);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
}

TEST(Lowerer, SequenceItemDuration) {
    const auto ir = lower("track { play [A4 :2, B4 :3]; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}

// ===========================================================================
// Chord
// ===========================================================================

TEST(Lowerer, ChordExpands) {
    const auto ir = lower("track { play (A4, C5); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, ir.tracks[0].events[1].start_beat);
}

TEST(Lowerer, ChordAdvancesByMaxDuration) {
    const auto ir = lower("track { play (A4 :3, C5 :1); play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    // Find B4 (midi=83).
    double b4_start = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
}

// ===========================================================================
// Let binding
// ===========================================================================

TEST(Lowerer, LetBinding) {
    const auto ir = lower("track { let n = 2; play A4 :n; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
}

TEST(Lowerer, GlobalLetVisibleInTrack) {
    const auto ir = lower("let vel = 4; track { play A4 :vel; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 4.0);
}

// ===========================================================================
// Control flow
// ===========================================================================

TEST(Lowerer, LoopUnrolls) {
    const auto ir = lower("track { loop (4) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 4u);
    for (std::size_t i = 0; i < 4; ++i) {
        EXPECT_DOUBLE_EQ(ir.tracks[0].events[i].start_beat, static_cast<double>(i));
    }
}

TEST(Lowerer, ForLoopUnrolls) {
    const auto ir = lower("track { for (let i = 0; i < 3; i = i + 1) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 3u);
}

TEST(Lowerer, ForLoopModuloAfterIntegerIncrement) {
    const auto ir = lower(R"(
        track {
            for (let i = 0; i < 4; i = i + 1) {
                if (i % 2 == 0) {
                    play A4;
                }
            }
        }
    )");
    EXPECT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(Lowerer, IfTakenBranch) {
    const auto ir = lower("track { if (true) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 1u);
}

TEST(Lowerer, IfElseNotTaken) {
    const auto ir = lower("track { if (false) { play A4; } else { play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 83);  // B4
}

TEST(Lowerer, IfConditionFalseNoEvents) {
    const auto ir = lower("track { if (false) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 0u);
}

// ===========================================================================
// Patterns
// ===========================================================================

TEST(Lowerer, GlobalPatternCall) {
    const auto ir = lower(
        "pattern riff() { play A4; play B4; }\n"
        "track { play riff(); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 83);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 1.0);
}

TEST(Lowerer, PatternWithArgs) {
    const auto ir = lower(
        "pattern note(n) { play A4 :n; }\n"
        "track { play note(3); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
}

// ===========================================================================
// Pattern parameter evaluation
// ===========================================================================

TEST(Lowerer, PatternSameLiteralArgTwice) {
    // Passing the same literal value to two params must bind both independently.
    const auto ir = lower(
        "pattern timed_pair(a, b) { play A4 :a; play B4 :b; }\n"
        "track { play timed_pair(3, 3); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    double a4_dur = -1.0, b4_dur = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 81) a4_dur = ev.duration_beats;
        if (ev.midi_note == 83) b4_dur = ev.duration_beats;
    }
    EXPECT_DOUBLE_EQ(a4_dur, 3.0);
    EXPECT_DOUBLE_EQ(b4_dur, 3.0);
}

TEST(Lowerer, PatternSameIdentArgTwice) {
    // Passing the same identifier to two params must resolve both from the
    // caller's scope, not the callee's partially-built scope.
    const auto ir = lower(
        "pattern timed_pair(a, b) { play A4 :a; play B4 :b; }\n"
        "track { let n = 2; play timed_pair(n, n); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    double a4_dur = -1.0, b4_dur = -1.0;
    for (const auto& ev : ir.tracks[0].events) {
        if (ev.midi_note == 81) a4_dur = ev.duration_beats;
        if (ev.midi_note == 83) b4_dur = ev.duration_beats;
    }
    EXPECT_DOUBLE_EQ(a4_dur, 2.0);
    EXPECT_DOUBLE_EQ(b4_dur, 2.0);
}

TEST(Lowerer, PatternNestedParamShadowing) {
    // When an inner pattern's param name matches an arg identifier from the
    // outer pattern, later args must still resolve in the outer scope.
    // extended_phrase(chord1, chord2, chord3):
    //   plays chord1 twice (dur 1), chord2 (dur 0.5), chord3 (dur 0.5)
    // phrase(chord1, chord2):
    //   calls extended_phrase(chord1, chord1, chord2)
    // Expected: events 0-1 are F2/A2/D3, event 2 is F2/A2/D3 at 0.5,
    //           event 3 is A2/C3/E3 at 0.5 — NOT F2/A2/D3 a fourth time.
    //
    // This compiler maps C4=72 (standard+12).
    // F2=53 A2=57 D3=62   A2=57 C3=60 E3=64
    const auto ir = lower(R"(
        tempo 200;
        track lead using piano {
            pattern extended_phrase(chord1, chord2, chord3) {
                play chord1;
                play chord1;
                play chord2 :0.5;
                play chord3 :0.5;
            }
            pattern phrase(chord1, chord2) {
                play extended_phrase(chord1, chord1, chord2);
            }
            play phrase((F2, A2, D3), (A2, C3, E3));
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    const auto& events = ir.tracks[0].events;

    // Collect midi notes per event group by start beat.
    auto notes_at = [&](double beat) {
        std::vector<int> notes;
        for (const auto& ev : events) {
            if (std::abs(ev.start_beat - beat) < 1e-9) notes.push_back(ev.midi_note);
        }
        std::ranges::sort(notes);
        return notes;
    };

    // First two beats: (F2=53, A2=57, D3=62)
    EXPECT_EQ(notes_at(0.0), (std::vector<int>{53, 57, 62}));
    EXPECT_EQ(notes_at(1.0), (std::vector<int>{53, 57, 62}));

    // Third beat (0.5 dur): still (F2, A2, D3) — second arg to extended_phrase is chord1
    EXPECT_EQ(notes_at(2.0), (std::vector<int>{53, 57, 62}));

    // Fourth beat (0.5 dur): (A2=57, C3=60, E3=64) — NOT F2/A2/D3
    EXPECT_EQ(notes_at(2.5), (std::vector<int>{57, 60, 64}));
}

// ===========================================================================
// Track defaults
// ===========================================================================

TEST(Lowerer, TrackDefaultInstrumentIsPiano) {
    const auto ir = lower("track { }");
    ASSERT_EQ(ir.tracks.size(), 1u);
    EXPECT_EQ(ir.tracks[0].instrument, Instrument::Piano);
}

TEST(Lowerer, TrackName) {
    const auto ir = lower("track melody { }");
    ASSERT_TRUE(ir.tracks[0].name.has_value());
    EXPECT_EQ(*ir.tracks[0].name, "melody");
}

TEST(Lowerer, TrackInstrument) {
    const auto ir = lower("track using guitar { }");
    EXPECT_EQ(ir.tracks[0].instrument, Instrument::Guitar);
}

TEST(Lowerer, EventsSortedByStartBeat) {
    // from-offset note at beat 5, then a normal note at beat 0.
    const auto ir = lower("track { play A4 from 5; play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_LE(ir.tracks[0].events[0].start_beat, ir.tracks[0].events[1].start_beat);
}

// ===========================================================================
// Drum notes
// ===========================================================================

TEST(Lowerer, DrumKick) {
    const auto ir = lower("track using drums { play kick; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 36);
}

// ===========================================================================
// Semantic errors
// ===========================================================================

TEST(Lowerer, UndeclaredVariable) {
    auto prog = parse("track { play x; }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

TEST(Lowerer, UndeclaredPattern) {
    auto prog = parse("track { play missing(); }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

TEST(Lowerer, LoopCountExceedsLimit) {
    auto prog = parse("track { loop (10001) { play A4; } }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

TEST(Lowerer, NoteOutOfMidiRange) {
    auto prog = parse("track { play C10; }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

TEST(Lowerer, DivisionByZero) {
    auto prog = parse("track { let n = 1 / 0; play A4 :n; }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

// ===========================================================================
// Ternary operator
// ===========================================================================

TEST(Lowerer, TernaryTrueCondition) {
    // True branch selected: A4 duration = 2, not 5.
    const auto ir = lower("track { let x = 1 == 1 ? 2 : 5; play A4 :x; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
}

TEST(Lowerer, TernaryFalseCondition) {
    // False branch selected: A4 duration = 5, not 2.
    const auto ir = lower("track { let x = 1 == 2 ? 2 : 5; play A4 :x; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 5.0);
}

TEST(Lowerer, TernaryAsPlayTarget) {
    // True condition: A4=81 is played, not B4=83.
    const auto ir = lower("track { play (1 == 1 ? A4 : B4); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
}

TEST(Lowerer, TernaryAsDuration) {
    // False condition: selects second branch (3), not first (1).
    const auto ir = lower("track { play A4 :(1 == 2 ? 1 : 3); }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
}

TEST(Lowerer, TernaryAsLoopCount) {
    // True condition: loops 3 times, not 1.
    const auto ir = lower("track { loop (1 == 1 ? 3 : 1) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 3u);
}

TEST(Lowerer, TernaryRightAssociative) {
    // 1==2 ? 1 : (1==1 ? 2 : 3) → false ? 1 : (true ? 2 : 3) → 2.
    const auto ir = lower("track { let x = 1 == 2 ? 1 : 1 == 1 ? 2 : 3; play A4 :x; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
}

TEST(Lowerer, TernaryInLetThenUsed) {
    // n = 4 via true branch; A4 plays for 4 beats.
    const auto ir = lower("track { let n = 1 > 0 ? 4 : 1; play A4 :n; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 4.0);
}

TEST(Lowerer, TernaryNonBoolCondition) {
    // Condition evaluates to int, not bool — semantic error.
    auto prog = parse("track { play (1 ? A4 : B4); }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

TEST(Lowerer, TernaryBranchTypeMismatch) {
    // Branches have different types (NoteVal vs int) — semantic error.
    auto prog = parse("track { let x = 1 == 1 ? A4 : 3; }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

// ===========================================================================
// Optional braces on control flow
// ===========================================================================

TEST(Lowerer, NoBraceIfTrue) {
    const auto ir = lower("track { if (1 == 1) play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
}

TEST(Lowerer, NoBraceIfFalse) {
    const auto ir = lower("track { if (1 == 2) play A4; }");
    EXPECT_EQ(ir.tracks[0].events.size(), 0u);
}

TEST(Lowerer, NoBraceIfElseTrue) {
    // Condition true: A4=81 plays, not B4=83.
    const auto ir = lower("track { if (1 == 1) play A4; else play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);
}

TEST(Lowerer, NoBraceIfElseFalse) {
    // Condition false: B4=83 plays, not A4=81.
    const auto ir = lower("track { if (1 == 2) play A4; else play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 83);
}

TEST(Lowerer, DanglingElseInnerFires) {
    // if (true) if (false) play A4; else play B4;
    // else binds to nearest (inner) if. Outer=true, inner=false → else fires → B4=83.
    const auto ir = lower("track { if (1 == 1) if (1 == 2) play A4; else play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 83);
}

TEST(Lowerer, DanglingElseOuterSkipped) {
    // if (false) if (true) play A4; else play B4;
    // Outer=false → entire inner if+else is skipped → no events.
    const auto ir = lower("track { if (1 == 2) if (1 == 1) play A4; else play B4; }");
    EXPECT_EQ(ir.tracks[0].events.size(), 0u);
}

TEST(Lowerer, ElseIfChainDepth3) {
    // All conditions false except last else → C4=72.
    const auto ir = lower("track { if (1 == 2) play A4; else if (1 == 3) play B4; else play C4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 72);
}

TEST(Lowerer, ElseIfChainMiddleTrue) {
    // Second condition true → B4=83.
    const auto ir = lower("track { if (1 == 2) play A4; else if (1 == 1) play B4; else play C4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 83);
}

TEST(Lowerer, NoBraceLoop) {
    // loop (3) play A4; → 3 events at beats 0, 1, 2.
    const auto ir = lower("track { loop (3) play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    std::vector<double> beats;
    for (const auto& ev : ir.tracks[0].events) beats.push_back(ev.start_beat);
    std::ranges::sort(beats);
    EXPECT_DOUBLE_EQ(beats[0], 0.0);
    EXPECT_DOUBLE_EQ(beats[1], 1.0);
    EXPECT_DOUBLE_EQ(beats[2], 2.0);
}

TEST(Lowerer, NoBraceFor) {
    // for (let i=0; i<3; i=i+1) play A4; → 3 events at beats 0, 1, 2.
    const auto ir = lower("track { for (let i = 0; i < 3; i = i + 1) play A4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    std::vector<double> beats;
    for (const auto& ev : ir.tracks[0].events) beats.push_back(ev.start_beat);
    std::ranges::sort(beats);
    EXPECT_DOUBLE_EQ(beats[0], 0.0);
    EXPECT_DOUBLE_EQ(beats[1], 1.0);
    EXPECT_DOUBLE_EQ(beats[2], 2.0);
}

TEST(Lowerer, NoBraceNested) {
    // if (true) loop (2) play A4; → 2 A4 events.
    const auto ir = lower("track { if (1 == 1) loop (2) play A4; }");
    EXPECT_EQ(ir.tracks[0].events.size(), 2u);
}

TEST(Lowerer, NoBraceAssignmentBody) {
    // if (true) x=4; → A4 plays with duration 4.
    const auto ir = lower("track { let x = 1; if (1 == 1) x = 4; play A4 :x; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 4.0);
}

// ===========================================================================
// Voice parallelism
// ===========================================================================

TEST(Lowerer, VoiceStartsAtOuterCursor) {
    // outer cursor = 2 when voice is declared → B4 placed at beat 2.
    const auto ir = lower("track { play A4; play A4; voice { play B4; } }");
    auto events = ir.tracks[0].events;
    ASSERT_EQ(events.size(), 3u);
    const auto it =
        std::find_if(events.begin(), events.end(), [](const auto& e) { return e.midi_note == 83; });  // B4=83
    ASSERT_NE(it, events.end());
    EXPECT_DOUBLE_EQ(it->start_beat, 2.0);
}

TEST(Lowerer, VoiceDoesNotAdvanceOuterCursor) {
    // voice starts at beat 1, plays C4:2 (ends at beat 3).
    // outer cursor stays at 1 → B4 also placed at beat 1.
    const auto ir = lower("track { play A4; voice { play C4 :2; } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    const auto b4 = std::find_if(ir.tracks[0].events.begin(), ir.tracks[0].events.end(), [](const auto& e) {
        return e.midi_note == 83;
    });  // B4=83
    ASSERT_NE(b4, ir.tracks[0].events.end());
    EXPECT_DOUBLE_EQ(b4->start_beat, 1.0);
}

TEST(Lowerer, VoiceFromZeroStartsAtBeatZero) {
    // outer cursor = 2, but voice from 0 overrides → B4 at beat 0.
    const auto ir = lower("track { play A4; play A4; voice from 0 { play B4; } }");
    const auto b4 = std::find_if(ir.tracks[0].events.begin(), ir.tracks[0].events.end(), [](const auto& e) {
        return e.midi_note == 83;
    });  // B4=83
    ASSERT_NE(b4, ir.tracks[0].events.end());
    EXPECT_DOUBLE_EQ(b4->start_beat, 0.0);
}

TEST(Lowerer, VoiceFromLiteralBeat) {
    // voice from 4 → B4 at beat 4.
    const auto ir = lower("track { voice from 4 { play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 4.0);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 83);  // B4=83
}

TEST(Lowerer, VoiceFromComputedExpr) {
    // let n = 3; voice from n → A4 at beat 3.
    const auto ir = lower("track { let n = 3; voice from n { play A4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 3.0);
}

TEST(Lowerer, VoiceInternalCursorAdvances) {
    // voice internal cursor advances normally: A4@0, B4@1.
    const auto ir = lower("track { voice { play A4; play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    auto events = ir.tracks[0].events;
    std::ranges::sort(events, [](const auto& a, const auto& b) { return a.start_beat < b.start_beat; });
    EXPECT_EQ(events[0].midi_note, 81);  // A4=81
    EXPECT_DOUBLE_EQ(events[0].start_beat, 0.0);
    EXPECT_EQ(events[1].midi_note, 83);  // B4=83
    EXPECT_DOUBLE_EQ(events[1].start_beat, 1.0);
}

TEST(Lowerer, TwoVoicesParallel) {
    // Two voices both start at outer cursor 0 → A4@0 and B4@0.
    const auto ir = lower("track { voice { play A4; } voice { play B4; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    const auto a4 = std::find_if(ir.tracks[0].events.begin(), ir.tracks[0].events.end(), [](const auto& e) {
        return e.midi_note == 81;
    });
    const auto b4 = std::find_if(ir.tracks[0].events.begin(), ir.tracks[0].events.end(), [](const auto& e) {
        return e.midi_note == 83;
    });
    ASSERT_NE(a4, ir.tracks[0].events.end());
    ASSERT_NE(b4, ir.tracks[0].events.end());
    EXPECT_DOUBLE_EQ(a4->start_beat, 0.0);
    EXPECT_DOUBLE_EQ(b4->start_beat, 0.0);
}

TEST(Lowerer, VoiceLoopWorks) {
    // loop (3) inside voice → 3 A4 events at beats 0, 1, 2.
    const auto ir = lower("track { voice { loop (3) { play A4; } } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 3u);
    std::vector<double> beats;
    for (const auto& ev : ir.tracks[0].events) beats.push_back(ev.start_beat);
    std::ranges::sort(beats);
    EXPECT_DOUBLE_EQ(beats[0], 0.0);
    EXPECT_DOUBLE_EQ(beats[1], 1.0);
    EXPECT_DOUBLE_EQ(beats[2], 2.0);
}

TEST(Lowerer, VoiceCallsTrackLevelPattern) {
    // Track-level pattern visible inside voice; outer cursor unchanged after voice.
    const auto ir = lower("track { pattern drone() { play A4; } voice { play drone(); } play B4; }");
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    const auto a4 = std::find_if(ir.tracks[0].events.begin(), ir.tracks[0].events.end(), [](const auto& e) {
        return e.midi_note == 81;
    });
    const auto b4 = std::find_if(ir.tracks[0].events.begin(), ir.tracks[0].events.end(), [](const auto& e) {
        return e.midi_note == 83;
    });
    ASSERT_NE(a4, ir.tracks[0].events.end());
    ASSERT_NE(b4, ir.tracks[0].events.end());
    EXPECT_DOUBLE_EQ(a4->start_beat, 0.0);
    EXPECT_DOUBLE_EQ(b4->start_beat, 0.0);
}

TEST(Lowerer, VoiceLocalPatternWorks) {
    // Pattern defined inside voice is callable within it.
    const auto ir = lower("track { voice { pattern p() { play A4; } play p(); } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 81);  // A4=81
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].start_beat, 0.0);
}

TEST(Lowerer, VoiceLocalPatternNotVisibleOutside) {
    // Pattern defined inside voice is NOT callable from outer track.
    auto prog = parse("track { voice { pattern p() { play A4; } } play p(); }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}

TEST(Lowerer, VoiceAndOuterNotesInterleaved) {
    // Outer note + 2 voice notes + outer note → 4 events total.
    const auto ir = lower("track { play A4; voice { play A4; play A4; } play B4; }");
    EXPECT_EQ(ir.tracks[0].events.size(), 4u);
}

TEST(Lowerer, VoiceLetWorks) {
    // let inside voice scopes correctly; A4 played with duration 3.
    const auto ir = lower("track { voice { let dur = 3; play A4 :dur; } }");
    ASSERT_EQ(ir.tracks[0].events.size(), 1u);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 3.0);
}

TEST(Lowerer, VoiceLetNotVisibleOutside) {
    // let declared inside voice is not visible in outer track.
    auto prog = parse("track { voice { let x = 5; } play A4 :x; }");
    ASSERT_NE(prog, nullptr);
    EXPECT_THROW(Lowerer{}.lower(*prog), SemanticError);
}
