#include "dsl/common/music/note.hpp"

#include <gtest/gtest.h>

using dsl::music::Accidental;
using dsl::music::Note;
using dsl::music::Pitch;

// C4 = MIDI 60 (common convention, other conventions exist but this is the most recognized one)

TEST(Note, MidiNumberNatural) {
    EXPECT_EQ((Note{Pitch::C, Accidental::Natural, 4}.midi_number()), 60);
    EXPECT_EQ((Note{Pitch::D, Accidental::Natural, 4}.midi_number()), 62);
    EXPECT_EQ((Note{Pitch::E, Accidental::Natural, 4}.midi_number()), 64);
    EXPECT_EQ((Note{Pitch::F, Accidental::Natural, 4}.midi_number()), 65);
    EXPECT_EQ((Note{Pitch::G, Accidental::Natural, 4}.midi_number()), 67);
    EXPECT_EQ((Note{Pitch::A, Accidental::Natural, 4}.midi_number()), 69);
    EXPECT_EQ((Note{Pitch::B, Accidental::Natural, 4}.midi_number()), 71);
}

TEST(Note, MidiNumberSharp) {
    EXPECT_EQ((Note{Pitch::C, Accidental::Sharp, 4}.midi_number()), 61);  // C#4
    EXPECT_EQ((Note{Pitch::F, Accidental::Sharp, 4}.midi_number()), 66);  // F#4
}

TEST(Note, MidiNumberFlat) {
    EXPECT_EQ((Note{Pitch::D, Accidental::Flat, 4}.midi_number()), 61);  // Db4 == C#4
    EXPECT_EQ((Note{Pitch::B, Accidental::Flat, 3}.midi_number()), 58);  // Bb3
}

TEST(Note, MidiNumberOctaveBoundary) {
    EXPECT_EQ((Note{Pitch::C, Accidental::Natural, 0}.midi_number()), 12);   // C0
    EXPECT_EQ((Note{Pitch::G, Accidental::Natural, 8}.midi_number()), 115);  // G8
}

TEST(Note, BuildsCorrectly) {
    constexpr Note n{Pitch::A, Accidental::Sharp, 3};
    EXPECT_EQ(n.pitch, Pitch::A);
    EXPECT_EQ(n.accidental, Accidental::Sharp);
    EXPECT_EQ(n.octave, 3);
}
