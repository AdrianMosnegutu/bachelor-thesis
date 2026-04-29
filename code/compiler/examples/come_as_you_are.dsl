tempo 240;
signature 4/4;

track using guitar {
    play D1;
    loop (3) {
        play [D1, D#1, E1:2];
        play [G1, E1, G1, E1, E1];
        play [D#1, D1, A1, D1, D1:2, A1];
    }
}

track using bass {
    play D1;

    loop (3) {
        play [D1, D#1, E1:6];
        play [E1:2 , D1:6];
    }
}

track using drums {
    pattern ok(play_crash) {
        play kick;
        play rest;

        if (play_crash) {
            play crash from 3;
        }

        play snare;
        play rest;

        play kick;
        play kick;
        play snare;
        loop (3) {play kick;}
        play snare;
        play kick;
        play rest;
        play kick;
        play snare;
    }

    loop (2) { play snare; }
    loop (4) { play ok(true); }
}