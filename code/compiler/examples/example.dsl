tempo 108;
signature 4/4;
key G major;

// ── Global motifs ────────────────────────────────────────────────
let verse_motif  = [G4:1, A4:0.5, B4:0.5, D5:1, B4:1];
let answer_motif = [A4:1, G4:0.5, F#4:0.5, E4:2];
let bridge_cell  = [B4:0.5, D5:0.5, B4:1, A4:1, G4:1];

// Play two motifs back-to-back as a phrase
pattern phrase(motif_a, motif_b) {
    play motif_a;
    play motif_b;
}

// ── Melody (piano) ───────────────────────────────────────────────
track melody using piano {
    // Local cadence pattern: run into a resolution chord
    pattern cadence(run) {
        play run;
        play (D4, F#4):2;
        play (G3, B3, D4):4;
    }

    // Intro – rising arpeggio, pause, then held fifth
    play [G4:1, rest:0.5, B4:0.5, D5:2];

    // Verse A – two linked motifs
    play phrase(verse_motif, answer_motif);

    // Verse B – decorated repeat (inline sequence as argument)
    let decorated = [F#4:0.5, G4:0.5, A4:1, B4:1, rest:1];
    play phrase(answer_motif, decorated);

    // Bridge – riff cell loops twice
    loop (2) {
        play bridge_cell;
    }

    // Outro – stepwise descent into a cadence
    play [D5:2, B4:1, G4:1];
    play cadence([E4:2, D4:1, G4:1]);
}

// ── Harmony (guitar) ─────────────────────────────────────────────
track harmony using guitar {
    // Intro pad – tonic whole note
    play (G3, B3, D4):4;

    // Two full I–vi–IV–V passes (G–Em–C–D)
    loop (2) {
        play (G3, B3, D4):4;
        play (E3, G3, B3):4;
        play (C3, E3, G3):4;
        play (D3, F#3, A3):4;
    }

    // Bridge – open power chord held for two bars
    play (G2, D3):8;

    // Outro cadence: vi–V–I
    play (E3, G3, B3):4;
    play (D3, F#3, A3):4;
    play (G2, B2, D3, G3):4;
}

// ── Bass ─────────────────────────────────────────────────────────
track bassline using bass {
    let root_walk = [G2:1, A2:1, B2:1, D3:1];

    // Intro – held root
    play G2:4;

    // Walking bass over two verse passes
    loop (2) {
        play root_walk;
        play [E2:1, G2:1, A2:0.5, B2:0.5, E2:2];
        play [C2:2, G2:2];
        play [D2:1, F#2:1, A2:1, D2:1];
    }

    // Bridge – alternating root/fifth in eighth notes
    for (let i = 0; i < 8; i = i + 1) {
        if (i % 2 == 0) {
            play G2:0.5;
        } else {
            play D2:0.5;
        }
    }

    // Outro – descend and resolve
    play [E2:2, D2:2, G2:4];
}

// ── Drums ────────────────────────────────────────────────────────
track percussion using drums {
    // Intro – sparse hi-hat only
    loop (4) {
        play hihat;
    }

    // Main groove: 16 steps – hi-hat every step, kick on 1 & 9, snare on 5 & 13
    pattern groove() {
        for (let i = 0; i < 16; i = i + 1) {
            play hihat from i;

            if (i % 8 == 0) {
                play kick from i;
            }

            if (i % 8 == 4) {
                play snare from i;
            }
        }
    }

    // Three bars of the main groove
    loop (3) {
        play groove();
    }

    // Final bar – crash accent on the downbeat
    play crash;
    play kick;
}
