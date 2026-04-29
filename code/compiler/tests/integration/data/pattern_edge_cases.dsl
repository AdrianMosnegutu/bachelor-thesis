tempo 120;
signature 4/4;

// Track 1: same pattern with an internal rest called twice in sequence.
// gapped(): A4(1) → rest(2) → B4(1) → dur=4.
// Call 1 at beat 0: A4@0, B4@3; cursor→4.
// Call 2 at beat 4: A4@4, B4@7; cursor→8.
// C4 at beat 8.
track pattern_called_twice {
    pattern gapped() {
        play A4;
        play rest :2;
        play B4;
    }
    play gapped();
    play gapped();
    play C4;
}

// Track 2: pattern whose body is entirely rests — no note events, but the outer
// cursor must still advance by the full rest duration.
// silent(): rest(3) → dur=3; A4 must land at beat 3.
track pattern_all_rests {
    pattern silent() {
        play rest :3;
    }
    play silent();
    play A4;
}

// Track 3: pattern that ends in a trailing rest.
// fade_out(): A4(1) → rest(2) → dur=3.
// Outer cursor must include the trailing silence; B4 at beat 3.
track pattern_trailing_rest {
    pattern fade_out() {
        play A4;
        play rest :2;
    }
    play fade_out();
    play B4;
}

// Track 4: three-level nesting where rests appear at both the inner and middle
// levels; verifies that gap-aware SeqVal reconstruction chains correctly.
//   base():       A4(1) → rest(1) → B4(1)           dur=3
//   wrap():       rest(1) → base()                   dur=4  (A4@1, B4@3)
//   outer_wrap(): wrap() → C4(1)                     dur=5  (A4@1, B4@3, C4@4)
// play outer_wrap(); play D4; → D4@5.
track deeply_nested {
    pattern base() {
        play A4;
        play rest :1;
        play B4;
    }
    pattern wrap() {
        play rest :1;
        play base();
    }
    pattern outer_wrap() {
        play wrap();
        play C4;
    }
    play outer_wrap();
    play D4;
}

// Track 5: pattern that takes a numeric duration parameter and applies it to notes.
// stretched(n): A4:n → B4:n → dur=2n. Call with n=2 → dur=4.
// A4@0 dur2, B4@2 dur2, C4@4.
track pattern_duration_param {
    pattern stretched(n) {
        play A4 :n;
        play B4 :n;
    }
    play stretched(2);
    play C4;
}
