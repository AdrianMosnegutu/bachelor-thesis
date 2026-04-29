tempo 120;
signature 4/4;

// Track 1: standalone rest with default duration advances cursor by 1 beat.
// Expected: A4 at beat 1.
track standalone_rest {
    play rest;
    play A4;
}

// Track 2: standalone rest with explicit duration advances cursor by that duration.
// Expected: A4 at beat 2.
track rest_explicit_duration {
    play rest :2;
    play A4;
}

// Track 3: rest inside a sequence uses default duration of 1 beat.
// Expected: A4 at beat 0, B4 at beat 2.
track sequence_rest_default {
    play [A4, rest, B4];
}

// Track 4: rest inside a sequence with explicit duration.
// Expected: A4 at beat 0, B4 at beat 3.
track sequence_rest_explicit {
    play [A4, rest :2, B4];
}

// Track 5: pattern containing a rest — the rest must shift subsequent notes
// and the outer cursor must advance by the full pattern span.
// Expected: A4 at beat 0 (dur 1), B4 at beat 3 (dur 1), C4 at beat 4 (dur 1).
track pattern_internal_rest {
    pattern gapped_phrase() {
        play A4;
        play rest :2;
        play B4;
    }

    play gapped_phrase();
    play C4;
}
