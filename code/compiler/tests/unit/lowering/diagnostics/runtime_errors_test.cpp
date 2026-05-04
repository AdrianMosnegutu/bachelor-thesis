#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"

using namespace dsl::testing::lowerer;

// -- MIDI event cap ------------------------------------------------------------

TEST(RuntimeErrors, ProgramUnder20kEventsIsValid) {
    // 100 events — well under the 20,000 total cap.
    const auto ir = lower_ok("track { loop (100) { play A4; } }");
    EXPECT_EQ(ir.tracks[0].events.size(), 100u);
}

TEST(RuntimeErrors, ProgramExceeding20kEventsTotalIsError) {
    // Two tracks, each looping 10,001 times = 20,002 total events.
    const auto res = lower_with_diagnostics(R"(
        track { loop (10001) { play A4; } }
        track { loop (10001) { play B4; } }
    )");
    EXPECT_TRUE(has_lowering_error(res.diagnostics.diagnostics(), "event"));
}

TEST(RuntimeErrors, SingleTrackWith20001EventsIsError) {
    const auto res = lower_with_diagnostics("track { loop (20001) { play A4; } }");
    EXPECT_TRUE(has_lowering_error(res.diagnostics.diagnostics(), "event"));
}

TEST(RuntimeErrors, Exactly20kEventsTotalIsValid) {
    // 20,000 events across two tracks (10,000 each).
    const auto ir = lower_ok(R"(
        track { loop (10000) { play A4; } }
        track { loop (10000) { play B4; } }
    )");
    EXPECT_EQ(ir.tracks[0].events.size(), 10000u);
    EXPECT_EQ(ir.tracks[1].events.size(), 10000u);
}

TEST(RuntimeErrors, InfiniteForLoopIsTerminatedByEventCap) {
    // for (;;) with no condition loops forever — the event cap must abort it.
    const auto res = lower_with_diagnostics("track { for (;;) { play A4; } }");
    EXPECT_TRUE(has_lowering_error(res.diagnostics.diagnostics(), "event"));
}

// -- Arithmetic runtime errors -------------------------------------------------

TEST(RuntimeErrors, DivisionByZeroAtRuntimeIsError) {
    const auto res = lower_with_diagnostics("track { play A4 :(1 / 0); }");
    EXPECT_TRUE(has_lowering_error(res.diagnostics.diagnostics(), "division"));
}

TEST(RuntimeErrors, ModuloByZeroAtRuntimeIsError) {
    const auto res = lower_with_diagnostics("track { play A4 :(1 % 0); }");
    EXPECT_TRUE(has_lowering_error(res.diagnostics.diagnostics(), "modulo"));
}

TEST(RuntimeErrors, DivisionByZeroViaVariableIsError) {
    const auto res = lower_with_diagnostics(R"(
        let d = 0;
        track { play A4 :(4 / d); }
    )");
    EXPECT_TRUE(has_lowering_error(res.diagnostics.diagnostics(), "division"));
}
