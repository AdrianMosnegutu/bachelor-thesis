tempo 200;
signature 4/4;

let BACKING_PIANO_START = 12;
let STRINGS_START = 9;

track lead_piano using piano {
    pattern yeah(chord1, chord2) {
        play chord1;
        play chord1;
        play chord1:0.5;
        play chord2:0.5;
    }

    pattern yeah2(chord1, chord2, chord3) {
        play chord1;
        play chord1;
        play chord2:0.5;
        play chord3:0.5;
    }

    for (let i = 0; i < 3; i = i + 1) {
        let l = 1;
        if (i < 2) { l = 3; }

        loop (3) { play [D3, D3:0.5]; }
        loop (l) { play D3:0.5; }
    }

    play [A2:0.5, C3:0.5];

    play yeah((F2, A2, D3), (A2, C3, E3));
    play yeah((A#2, D3, F3), (G3, D3));
    play yeah2((E3, C3, A2), (D3, A2), (C3, G2));

    play (A2, C3):0.5;
    play (A2, D3);
    play rest:0.5;

    play [A2:0.5, C3:0.5];

    play yeah2((F2, A#2, D3), (A#2, D3), (A#2, E3));
    play yeah2((A2, C3, F3), (F3, C3), (C3, G3));
    play yeah2((A2, C3, E3), (A2, D3), (G2, C3));

    play (F2, A2, D3);
    play rest;
}

track backing_piano using piano {
    pattern phrase(chord1, chord2) {
        play chord1;
        play chord1:0.5;
        play chord1;
        play chord2:0.5;
    }

    pattern phrase_simple(chord) {
        play phrase(chord, chord);
    }

    pattern start() {
        play (D0, D1):3;
        play (D0, D1):1.5;
        play (D0, D1):1.5;

        play phrase((D1, D2), (C1, C2));
        play phrase_simple((A#0, A#1));
        play phrase_simple((A0, A1));
        play phrase_simple((D1, D2));

        play phrase_simple((A#0, A#1));
        play phrase_simple((A#0, A#1));
        play phrase_simple((A0, A1));
        play (D1, D2);
    }

    play start() from BACKING_PIANO_START;
}

track strings using violin {
    pattern start() {
        play [D3:3, D3:3, A#3:1.5, A3, A3:0.5];
        play [D3:2, D3:0.5, E3:0.5, F3:1.5, F3:0.75, G3:0.75, E3:2];
        play [D3:0.5, C3:0.5, C3:0.5, D3:1.5];

        play [A2:0.5, C3:0.5, D3:2];
        play [D3:0.5, E3:0.5, F3:2];
        play [F3:0.5, G3:0.5, E3:2];
        play [D3:0.5, C3:0.5, D3:0.75];
    }

    play start() from STRINGS_START;
}
