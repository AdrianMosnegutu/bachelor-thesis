tempo 120;
signature 4/4;

// Track 1: trailing rest at the end of a sequence must advance the cursor.
// [A4, rest:3] occupies 4 beats; B4 must land at beat 4.
track seq_trailing_rest {
    play [A4, rest :3];
    play B4;
}

// Track 2: sequence that contains only rests — no note events, but cursor
// must advance by the sum of all rest durations (2+3 = 5 beats).
track seq_all_rests {
    play [rest :2, rest :3];
    play A4;
}

// Track 3: chord followed by a trailing rest inside a sequence.
// (A4,C5) has default duration 1; rest:2 adds 2 more beats → total 3.
// B4 must land at beat 3.
track seq_chord_trailing_rest {
    play [(A4, C5), rest :2];
    play B4;
}

// Track 4: note sandwiched between a leading rest and a trailing rest.
// [rest:1, A4, rest:2] → A4@1, total 4 beats; B4 must land at beat 4.
track seq_sandwich {
    play [rest :1, A4, rest :2];
    play B4;
}

// Track 5: two consecutive sequences each ending in a trailing rest.
// [A4, rest:3] → 4 beats; [B4, rest:3] → 4 beats; C4@8.
track seq_multi_trailing {
    play [A4, rest :3];
    play [B4, rest :3];
    play C4;
}
