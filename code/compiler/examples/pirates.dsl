tempo 200;
signature 4/4;

let BACKING_PIANO_START = 12;
let STRINGS_START = 9;

track lead_piano using piano {
   pattern foo(chord1, chord2, chord3) {
      play [chord1, chord1, chord2:0.5, chord3:0.5];
   }

   pattern foo(chord1, chord2) {
      play foo(chord1, chord1, chord2);
   }

   for (let i = 0; i < 3; i = i + 1) {
      loop (3) play [D4, D4:0.5];
      loop (i == 2 ? 1 : 3) play D4:0.5;
   }

   play [A3:0.5, C4:0.5];

   play foo((F3, A3, D4), (A3, C4, E4));
   play foo((A#3, D4, F4), (G4, D4));
   play foo((E4, C4, A3), (D4, A3), (C4, G3));

   play [(A3, C4):0.5, (A3, D4), rest:0.5];

   play [A3:0.5, C4:0.5];

   play foo((F3, A#3, D4), (A#3, D4), (A#3, E4));
   play foo((A3, C4, F4), (F4, C4), (C4, G4));
   play foo((A3, C4, E4), (A3, D4), (G3, C4));

   play (F3, A3, D4);
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
      let chord = (D1, D2);
      play [chord:3, chord:1.5, chord:1.5];

      play phrase((D2, D3), (C2, C3));
      play phrase_simple((A#1, A#2));
      play phrase_simple((A1, A2));
      play phrase_simple((D2, D3));

      play phrase_simple((A#1, A#2));
      play phrase_simple((A#1, A#2));
      play phrase_simple((A1, A2));
      play (D2, D3);
   }

   play start() from BACKING_PIANO_START;
}

track strings using violin {
   pattern start() {
      play [D4:3, D4:3, A#4:1.5, A4, A4:0.5];
      play [D4:2, D4:0.5, E4:0.5, F4:1.5, F4:0.75, G4:0.75, E4:2];
      play [D4:0.5, C4:0.5, C4:0.5, D4:1.5];

      play [A3:0.5, C4:0.5, D4:2];
      play [D4:0.5, E4:0.5, F4:2];
      play [F4:0.5, G4:0.5, E4:2];
      play [D4:0.5, C4:0.5, D4:0.75];
   }

   play start() from STRINGS_START;
}
