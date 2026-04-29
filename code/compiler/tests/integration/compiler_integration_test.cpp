#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "dsl/core/ast/program.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/program.hpp"
#include "parser.hpp"

// -- Flex interface ----------------------------------------------------------
struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void scanner_reset();

// -- Helpers -----------------------------------------------------------------
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

dsl::ir::ProgramIR compile_file(const std::string& path) {
    std::ifstream file(path);
    EXPECT_TRUE(file.is_open()) << "Could not open: " << path;
    std::ostringstream ss;
    ss << file.rdbuf();
    const std::string src = ss.str();

    ParseGuard guard(src);
    auto program = std::make_unique<dsl::ast::Program>();
    dsl::Location loc;
    dsl::frontend::Parser parser{loc, *program};
    EXPECT_EQ(parser.parse(), 0) << "Parse failed for: " << path;
    return dsl::ir::Lowerer{}.lower(*program);
}

std::vector<int> notes_at(const dsl::ir::TrackIR& track, double beat) {
    std::vector<int> notes;
    for (const auto& ev : track.events) {
        if (std::abs(ev.start_beat - beat) < 1e-9) notes.push_back(ev.midi_note);
    }
    std::sort(notes.begin(), notes.end());
    return notes;
}

const std::string REST_DSL = INTEGRATION_DATA_DIR "/rest_scenarios.dsl";
const std::string NESTED_DSL = INTEGRATION_DATA_DIR "/nested_patterns.dsl";
const std::string SEQ_DSL = INTEGRATION_DATA_DIR "/sequence_cursor.dsl";
const std::string PATTERN_DSL = INTEGRATION_DATA_DIR "/pattern_edge_cases.dsl";
const std::string CONTROL_DSL = INTEGRATION_DATA_DIR "/control_flow.dsl";

}  // namespace

// ===========================================================================
// rest_scenarios.dsl
// ===========================================================================

TEST(Integration, RestStandaloneAdvancesCursor) {
    const auto ir = compile_file(REST_DSL);
    ASSERT_GE(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];  // standalone_rest
    ASSERT_EQ(track.events.size(), 1u);
    EXPECT_EQ(track.events[0].midi_note, 81);  // A4
    EXPECT_DOUBLE_EQ(track.events[0].start_beat, 1.0);
}

TEST(Integration, RestExplicitDurationAdvancesCursor) {
    const auto ir = compile_file(REST_DSL);
    ASSERT_GE(ir.tracks.size(), 2u);
    const auto& track = ir.tracks[1];  // rest_explicit_duration
    ASSERT_EQ(track.events.size(), 1u);
    EXPECT_EQ(track.events[0].midi_note, 81);  // A4
    EXPECT_DOUBLE_EQ(track.events[0].start_beat, 2.0);
}

TEST(Integration, SequenceRestDefault) {
    const auto ir = compile_file(REST_DSL);
    ASSERT_GE(ir.tracks.size(), 3u);
    const auto& track = ir.tracks[2];  // sequence_rest_default
    ASSERT_EQ(track.events.size(), 2u);
    EXPECT_EQ(track.events[0].midi_note, 81);  // A4
    EXPECT_DOUBLE_EQ(track.events[0].start_beat, 0.0);
    EXPECT_EQ(track.events[1].midi_note, 83);  // B4
    EXPECT_DOUBLE_EQ(track.events[1].start_beat, 2.0);
}

TEST(Integration, SequenceRestExplicit) {
    const auto ir = compile_file(REST_DSL);
    ASSERT_GE(ir.tracks.size(), 4u);
    const auto& track = ir.tracks[3];  // sequence_rest_explicit
    ASSERT_EQ(track.events.size(), 2u);
    EXPECT_EQ(track.events[0].midi_note, 81);  // A4
    EXPECT_DOUBLE_EQ(track.events[0].start_beat, 0.0);
    EXPECT_EQ(track.events[1].midi_note, 83);  // B4
    EXPECT_DOUBLE_EQ(track.events[1].start_beat, 3.0);
}

TEST(Integration, PatternInternalRestPreservesGap) {
    const auto ir = compile_file(REST_DSL);
    ASSERT_GE(ir.tracks.size(), 5u);
    const auto& track = ir.tracks[4];  // pattern_internal_rest
    ASSERT_EQ(track.events.size(), 3u);

    double a4_start = -1.0, b4_start = -1.0, c4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;  // A4
        if (ev.midi_note == 83) b4_start = ev.start_beat;  // B4
        if (ev.midi_note == 72) c4_start = ev.start_beat;  // C4
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
    EXPECT_DOUBLE_EQ(c4_start, 4.0);
}

// ===========================================================================
// nested_patterns.dsl
// ===========================================================================

TEST(Integration, NestedPatternSimpleTwoLevel) {
    const auto ir = compile_file(NESTED_DSL);
    ASSERT_GE(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];  // simple_nesting
    ASSERT_EQ(track.events.size(), 2u);

    double a4_start = -1.0, b4_start = -1.0;
    double a4_dur = -1.0, b4_dur = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) {
            a4_start = ev.start_beat;
            a4_dur = ev.duration_beats;
        }
        if (ev.midi_note == 83) {
            b4_start = ev.start_beat;
            b4_dur = ev.duration_beats;
        }
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(a4_dur, 3.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
    EXPECT_DOUBLE_EQ(b4_dur, 3.0);
}

TEST(Integration, NestedPatternParamShadowing) {
    const auto ir = compile_file(NESTED_DSL);
    ASSERT_GE(ir.tracks.size(), 2u);
    const auto& track = ir.tracks[1];  // nested_param_shadowing

    // Compiler MIDI: C4=72 (standard+12).
    // F2=53  A2=57  D3=62   C3=60  E3=64
    const std::vector<int> chord1 = {53, 57, 62};  // F2 A2 D3
    const std::vector<int> chord2 = {57, 60, 64};  // A2 C3 E3

    EXPECT_EQ(notes_at(track, 0.0), chord1);
    EXPECT_EQ(notes_at(track, 1.0), chord1);
    EXPECT_EQ(notes_at(track, 2.0), chord1);
    EXPECT_EQ(notes_at(track, 2.5), chord2);  // fails before fix-evaluate-call-scope
}

// ===========================================================================
// sequence_cursor.dsl
// ===========================================================================

TEST(Integration, SeqTrailingRestAdvancesCursor) {
    const auto ir = compile_file(SEQ_DSL);
    ASSERT_GE(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];  // seq_trailing_rest
    // A4=81 at beat 0; B4=83 must land at beat 4 (1+3).
    double a4_start = -1.0, b4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(b4_start, 4.0);
}

TEST(Integration, SeqAllRestsAdvancesCursor) {
    const auto ir = compile_file(SEQ_DSL);
    ASSERT_GE(ir.tracks.size(), 2u);
    const auto& track = ir.tracks[1];  // seq_all_rests
    // Sequence is only rests (2+3=5 beats); A4=81 must land at beat 5.
    ASSERT_EQ(track.events.size(), 1u);
    EXPECT_EQ(track.events[0].midi_note, 81);
    EXPECT_DOUBLE_EQ(track.events[0].start_beat, 5.0);
}

TEST(Integration, SeqChordTrailingRest) {
    const auto ir = compile_file(SEQ_DSL);
    ASSERT_GE(ir.tracks.size(), 3u);
    const auto& track = ir.tracks[2];  // seq_chord_trailing_rest
    // (A4=81, C5=84) chord at beat 0 dur 1; rest:2 → total 3 beats; B4=83@3.
    EXPECT_EQ(notes_at(track, 0.0), (std::vector<int>{81, 84}));
    EXPECT_EQ(notes_at(track, 3.0), (std::vector<int>{83}));
}

TEST(Integration, SeqSandwichRestsAroundNote) {
    const auto ir = compile_file(SEQ_DSL);
    ASSERT_GE(ir.tracks.size(), 4u);
    const auto& track = ir.tracks[3];  // seq_sandwich
    // [rest:1, A4, rest:2] → A4=81 at beat 1, total 4 beats; B4=83 at beat 4.
    double a4_start = -1.0, b4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 1.0);
    EXPECT_DOUBLE_EQ(b4_start, 4.0);
}

TEST(Integration, SeqMultiTrailingRestsAccumulate) {
    const auto ir = compile_file(SEQ_DSL);
    ASSERT_GE(ir.tracks.size(), 5u);
    const auto& track = ir.tracks[4];  // seq_multi_trailing
    // [A4,rest:3]=4 beats, [B4,rest:3]=4 beats, C4=72 at beat 8.
    double a4_start = -1.0, b4_start = -1.0, c4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
        if (ev.midi_note == 72) c4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(b4_start, 4.0);
    EXPECT_DOUBLE_EQ(c4_start, 8.0);
}

// ===========================================================================
// pattern_edge_cases.dsl
// ===========================================================================

TEST(Integration, PatternCalledTwiceCursorAccumulates) {
    const auto ir = compile_file(PATTERN_DSL);
    ASSERT_GE(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];  // pattern_called_twice
    // gapped(): A4(1)+rest(2)+B4(1) = 4 beats.
    // Call 1: A4=81@0, B4=83@3. Call 2: A4@4, B4@7. C4=72@8.
    ASSERT_EQ(track.events.size(), 5u);
    std::vector<double> a4_beats, b4_beats;
    double c4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_beats.push_back(ev.start_beat);
        if (ev.midi_note == 83) b4_beats.push_back(ev.start_beat);
        if (ev.midi_note == 72) c4_start = ev.start_beat;
    }
    ASSERT_EQ(a4_beats.size(), 2u);
    ASSERT_EQ(b4_beats.size(), 2u);
    std::sort(a4_beats.begin(), a4_beats.end());
    std::sort(b4_beats.begin(), b4_beats.end());
    EXPECT_DOUBLE_EQ(a4_beats[0], 0.0);
    EXPECT_DOUBLE_EQ(a4_beats[1], 4.0);
    EXPECT_DOUBLE_EQ(b4_beats[0], 3.0);
    EXPECT_DOUBLE_EQ(b4_beats[1], 7.0);
    EXPECT_DOUBLE_EQ(c4_start, 8.0);
}

TEST(Integration, PatternAllRestsAdvancesOuterCursor) {
    const auto ir = compile_file(PATTERN_DSL);
    ASSERT_GE(ir.tracks.size(), 2u);
    const auto& track = ir.tracks[1];  // pattern_all_rests
    // silent(): rest(3). No note events from the pattern; A4=81 must be at beat 3.
    ASSERT_EQ(track.events.size(), 1u);
    EXPECT_EQ(track.events[0].midi_note, 81);
    EXPECT_DOUBLE_EQ(track.events[0].start_beat, 3.0);
}

TEST(Integration, PatternTrailingRestIncludedInCursor) {
    const auto ir = compile_file(PATTERN_DSL);
    ASSERT_GE(ir.tracks.size(), 3u);
    const auto& track = ir.tracks[2];  // pattern_trailing_rest
    // fade_out(): A4(1)+rest(2) = 3 beats; B4=83 must be at beat 3.
    ASSERT_EQ(track.events.size(), 2u);
    double a4_start = -1.0, b4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
}

TEST(Integration, DeeplyNestedPatternsWithRests) {
    const auto ir = compile_file(PATTERN_DSL);
    ASSERT_GE(ir.tracks.size(), 4u);
    const auto& track = ir.tracks[3];  // deeply_nested
    // base: A4@0,rest,B4@2 (dur 3)
    // wrap: rest(1)+base → A4@1, B4@3 (dur 4)
    // outer_wrap: wrap()+C4 → A4@1, B4@3, C4=72@4 (dur 5)
    // play outer_wrap(); play D4=74; → D4@5.
    ASSERT_EQ(track.events.size(), 4u);
    double a4_start = -1.0, b4_start = -1.0, c4_start = -1.0, d4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
        if (ev.midi_note == 72) c4_start = ev.start_beat;
        if (ev.midi_note == 74) d4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 1.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
    EXPECT_DOUBLE_EQ(c4_start, 4.0);
    EXPECT_DOUBLE_EQ(d4_start, 5.0);
}

TEST(Integration, PatternDurationFromParam) {
    const auto ir = compile_file(PATTERN_DSL);
    ASSERT_GE(ir.tracks.size(), 5u);
    const auto& track = ir.tracks[4];  // pattern_duration_param
    // stretched(2): A4=81@0 dur2, B4=83@2 dur2; C4=72@4.
    ASSERT_EQ(track.events.size(), 3u);
    double a4_start = -1.0, b4_start = -1.0, c4_start = -1.0;
    double a4_dur = -1.0, b4_dur = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) {
            a4_start = ev.start_beat;
            a4_dur = ev.duration_beats;
        }
        if (ev.midi_note == 83) {
            b4_start = ev.start_beat;
            b4_dur = ev.duration_beats;
        }
        if (ev.midi_note == 72) c4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(a4_dur, 2.0);
    EXPECT_DOUBLE_EQ(b4_start, 2.0);
    EXPECT_DOUBLE_EQ(b4_dur, 2.0);
    EXPECT_DOUBLE_EQ(c4_start, 4.0);
}

// ===========================================================================
// control_flow.dsl
// ===========================================================================

TEST(Integration, FromOffsetDoesNotAdvanceCursor) {
    const auto ir = compile_file(CONTROL_DSL);
    ASSERT_GE(ir.tracks.size(), 1u);
    const auto& track = ir.tracks[0];  // from_no_cursor_advance
    // A4=81 from 5 → @5 but cursor stays 0. B4=83@0, C4=72@1.
    ASSERT_EQ(track.events.size(), 3u);
    double a4_start = -1.0, b4_start = -1.0, c4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_start = ev.start_beat;
        if (ev.midi_note == 83) b4_start = ev.start_beat;
        if (ev.midi_note == 72) c4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 5.0);
    EXPECT_DOUBLE_EQ(b4_start, 0.0);
    EXPECT_DOUBLE_EQ(c4_start, 1.0);
}

TEST(Integration, LetVariableUsedAsDuration) {
    const auto ir = compile_file(CONTROL_DSL);
    ASSERT_GE(ir.tracks.size(), 2u);
    const auto& track = ir.tracks[1];  // let_as_duration
    // let n=3; play A4 :n → A4=81@0 dur3; play B4=83 → @3.
    ASSERT_EQ(track.events.size(), 2u);
    double a4_start = -1.0, b4_start = -1.0;
    double a4_dur = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) {
            a4_start = ev.start_beat;
            a4_dur = ev.duration_beats;
        }
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    EXPECT_DOUBLE_EQ(a4_start, 0.0);
    EXPECT_DOUBLE_EQ(a4_dur, 3.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
}

TEST(Integration, LoopAdvancesCursorEachIteration) {
    const auto ir = compile_file(CONTROL_DSL);
    ASSERT_GE(ir.tracks.size(), 3u);
    const auto& track = ir.tracks[2];  // loop_advances_cursor
    // loop(3){ play A4; } → A4=81 at beats 0,1,2; play B4=83 → @3.
    ASSERT_EQ(track.events.size(), 4u);
    std::vector<double> a4_beats;
    double b4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_beats.push_back(ev.start_beat);
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    std::sort(a4_beats.begin(), a4_beats.end());
    ASSERT_EQ(a4_beats.size(), 3u);
    EXPECT_DOUBLE_EQ(a4_beats[0], 0.0);
    EXPECT_DOUBLE_EQ(a4_beats[1], 1.0);
    EXPECT_DOUBLE_EQ(a4_beats[2], 2.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
}

TEST(Integration, ForLoopAdvancesCursorEachIteration) {
    const auto ir = compile_file(CONTROL_DSL);
    ASSERT_GE(ir.tracks.size(), 4u);
    const auto& track = ir.tracks[3];  // for_loop_advances_cursor
    // for(let i=0;i<3;i=i+1){ play A4; } → A4=81 at 0,1,2; B4=83@3.
    ASSERT_EQ(track.events.size(), 4u);
    std::vector<double> a4_beats;
    double b4_start = -1.0;
    for (const auto& ev : track.events) {
        if (ev.midi_note == 81) a4_beats.push_back(ev.start_beat);
        if (ev.midi_note == 83) b4_start = ev.start_beat;
    }
    std::sort(a4_beats.begin(), a4_beats.end());
    ASSERT_EQ(a4_beats.size(), 3u);
    EXPECT_DOUBLE_EQ(a4_beats[0], 0.0);
    EXPECT_DOUBLE_EQ(a4_beats[1], 1.0);
    EXPECT_DOUBLE_EQ(a4_beats[2], 2.0);
    EXPECT_DOUBLE_EQ(b4_start, 3.0);
}
