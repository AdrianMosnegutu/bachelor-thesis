#include <gtest/gtest.h>

#include "support/lowerer_test_support.hpp"
using namespace dsl::testing::lowerer;

TEST(LetBinding, GlobalAndLocalBindingsDriveDurations) {
    const auto ir = lower_ok(R"(
        let global_dur = 2;
        track {
            let local_dur = global_dur + 1;
            play A4 :global_dur;
            play B4 :local_dur;
        }
    )");

    ASSERT_EQ(ir.tracks.size(), 1u);
    ASSERT_EQ(ir.tracks[0].events.size(), 2u);
    EXPECT_EQ(ir.tracks[0].events[0].midi_note, 69);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[0].duration_beats, 2.0);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].start_beat, 2.0);
    EXPECT_EQ(ir.tracks[0].events[1].midi_note, 71);
    EXPECT_DOUBLE_EQ(ir.tracks[0].events[1].duration_beats, 3.0);
}
