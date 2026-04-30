#include "dsl/music/note.hpp"

#include <gtest/gtest.h>

using dsl::music::Accidental;
using dsl::music::Note;
using dsl::music::Pitch;

// C4 = MIDI 72 (12*(4+2) + 0 + 0)
TEST(Note, MidiNumberNatural) {
    EXPECT_EQ((Note{Pitch::C, Accidental::Natural, 4}.midi_number()), 72);
    EXPECT_EQ((Note{Pitch::D, Accidental::Natural, 4}.midi_number()), 74);
    EXPECT_EQ((Note{Pitch::E, Accidental::Natural, 4}.midi_number()), 76);
    EXPECT_EQ((Note{Pitch::F, Accidental::Natural, 4}.midi_number()), 77);
    EXPECT_EQ((Note{Pitch::G, Accidental::Natural, 4}.midi_number()), 79);
    EXPECT_EQ((Note{Pitch::A, Accidental::Natural, 4}.midi_number()), 81);
    EXPECT_EQ((Note{Pitch::B, Accidental::Natural, 4}.midi_number()), 83);
}

TEST(Note, MidiNumberSharp) {
    EXPECT_EQ((Note{Pitch::C, Accidental::Sharp, 4}.midi_number()), 73);  // C#4
    EXPECT_EQ((Note{Pitch::F, Accidental::Sharp, 4}.midi_number()), 78);  // F#4
}

TEST(Note, MidiNumberFlat) {
    EXPECT_EQ((Note{Pitch::D, Accidental::Flat, 4}.midi_number()), 73);  // Db4 == C#4
    EXPECT_EQ((Note{Pitch::B, Accidental::Flat, 3}.midi_number()), 70);  // Bb3 (C4=72 convention)
}

TEST(Note, MidiNumberOctaveBoundary) {
    EXPECT_EQ((Note{Pitch::C, Accidental::Natural, 0}.midi_number()), 24);   // C0 = MIDI 24
    EXPECT_EQ((Note{Pitch::G, Accidental::Natural, 8}.midi_number()), 127);  // G8 = MIDI 127
}

TEST(Note, InheritsFromPitchClass) {
    constexpr Note n{Pitch::A, Accidental::Sharp, 3};
    EXPECT_EQ(n.pitch, Pitch::A);
    EXPECT_EQ(n.accidental, Accidental::Sharp);
    EXPECT_EQ(n.octave, 3);
}
