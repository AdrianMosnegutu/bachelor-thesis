tempo 300;  // beats per minute

/* To be played with the right hand. */
track lead using piano {
    // Alternates between two notes for a set number of times
    pattern alternate(note1, note2, len) {
        for (let i = 0; i < len; i = i + 1)
            play (i % 2 == 0 ? note1 : note2);
    }

    // Initial phrase with which the composition starts
    pattern main_phrase(last_sequence) {
        play alternate(E4, D#4, 5);
        play [B3, D4, C4, A3:3];
        play [C3, E3, A3, B3:3];
        play last_sequence;
    }

    // The followup after the main phrase
    pattern followup_phrase() {
        play [B3, C4, D4, E4:3];
        play [G3, F4, E4, D4:3];
        play [F3, E4, D4, C4:3];
        play [E3, D4, C4, B3:3];
    }

    pattern main() {
        for (let i = 0; i < 2; i = i + 1) {
            play main_phrase([E3, G#3, B3, C4:3, E3]);
            play main_phrase([E3, C4, B3, A3:(i == 1 ? 3 : 4)]);
        }

        play followup_phrase();

        play alternate(E3, E4, 4);
        play [E4, E5];
        play alternate(D#4, E4, 7);
    }

    loop (2) play main();
}

/* To be played with the left hand. */
track backing using piano {
    pattern phrase(rest_time_last) { 
        play [A1, E2, A2, rest:3];
        play [E1, E2, G#2, rest:3];
        play [A1, E2, A2, rest:rest_time_last];
    }

    pattern next_phrase() {
        play [C2, G2, C3, rest:3];
        play [G1, G2, B2, rest:3];
        play [A1, E2, A2, rest:3];
        play [E1, E2, E3, rest:3];
    }

    pattern main(padding_before) {
        play rest:padding_before;
        for (let i = 0; i < 4; i = i + 1)
            play phrase(i == 3 ? 3 : 9);
        play next_phrase();
    }

    let pause = 8;

    play main(pause);
    play main((pause + 1) * 2);
}

