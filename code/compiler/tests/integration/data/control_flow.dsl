tempo 120;
signature 4/4;

// Track 1: `from` positions a note at an absolute beat without advancing the
// cursor. Subsequent plays therefore start from the pre-from cursor position.
// play A4 from 5 → A4@5, cursor stays 0.
// play B4 → B4@0, cursor→1.
// play C4 → C4@1.
track from_no_cursor_advance {
    play A4 from 5;
    play B4;
    play C4;
}

// Track 2: `let` variable used as a note duration.
// let n = 3; play A4 :n → A4@0 dur3; play B4 → B4@3.
track let_as_duration {
    let n = 3;
    play A4 :n;
    play B4;
}

// Track 3: `loop(n)` repeats its body n times; the cursor advances each
// iteration so consecutive iterations produce consecutive notes.
// loop(3){ play A4; } → A4@0, A4@1, A4@2; play B4 → B4@3.
track loop_advances_cursor {
    loop (3) { play A4; }
    play B4;
}

// Track 4: C-style `for` loop with let-init and step; same cursor semantics
// as loop — each body execution advances the cursor.
// for(let i=0; i<3; i=i+1){ play A4; } → A4@0,1,2; play B4 → B4@3.
track for_loop_advances_cursor {
    for (let i = 0; i < 3; i = i + 1) { play A4; }
    play B4;
}
