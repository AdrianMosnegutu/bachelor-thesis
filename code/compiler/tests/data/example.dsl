tempo 130;
signature 4/4;

let GLOBAL_VAR = 10;
let GLOBAL_SEQ = [A4, rest, B4:2, C4:3];
let GLOBAL_CHORD = (A3:2, B2, C3:3);

pattern global_pattern() {}

track {
    pattern intro_melody(my_sequence) {
        play my_sequence;
        play [A4, B4, A4, G4];
    }
    let my_seq = [A2, B2:2, rest:3, C#3:1];
    play (A3, C#3);
    play intro_melody(my_seq) from 1;
}

track bassline using bass {
    for (let i = 0; i < 4; i = i + 1) {
        if (i < 2) { play [E2, E2, B2, B2]; }
        else { play [A2, A2, G2, G2]; }
    }
    loop (3) {}
    play (E2, B2):16;
}

track bassline using bass {}

track using drums {
    pattern rock_beat() {
        for (let i = 0; i < 16; i = i + 1) {
            play hihat;
            let my_var = 123;
            my_var = 2345;
        }
    }
    play rock_beat() from 17;
}
