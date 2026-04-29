tempo 120;
signature 4/4;

// Track 1: simple two-level pattern call — no param name shadowing.
// outer calls inner(n, n) where inner has params (a, b).
// Expected: A4 at beat 0 dur 3, B4 at beat 3 dur 3.
track simple_nesting {
    pattern timed_pair(a, b) {
        play A4 :a;
        play B4 :b;
    }

    pattern doubled(n) {
        play timed_pair(n, n);
    }

    play doubled(3);
}

// Track 2: nested pattern call where the inner param name matches the outer
// param name — the scope pollution bug.
// extended_phrase(chord1, chord2, chord3) plays chord1 twice, chord2 at 0.5,
// chord3 at 0.5. phrase(chord1, chord2) calls extended_phrase(chord1, chord1, chord2).
// With the bug: chord3 resolves to chord2 from extended_phrase's own scope
// (which was set to chord1), so (F2,A2,D3) plays four times.
// Expected: beats 0 and 1 → F2/A2/D3, beat 2 → F2/A2/D3, beat 2.5 → A2/C3/E3.
// Compiler MIDI (C4=72, standard+12):
//   F2=53  A2=57  D3=62   A2=57  C3=60  E3=64
track nested_param_shadowing {
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
