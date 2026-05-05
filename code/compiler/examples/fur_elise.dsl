tempo 300;  // beats per minute
signature 4/4;

/* To be played with the right hand. */
track lead using piano {
   // Alternates between two notes for a set number of times
   pattern alternate(note1, note2, len) {
      for (let i = 0; i < len; i = i + 1)
         play (i % 2 == 0 ? note1 : note2);
   }

   // Initial phrase with which the composition starts
   pattern main_phrase(last_sequence) {
      play alternate(E5, D#5, 5);
      play [B4, D5, C5, A4:3];
      play [C4, E4, A4, B4:3];
      play last_sequence;
   }

   // The followup after the main phrase
   pattern followup_phrase() {
      play [B4, C5, D5, E5:3];
      play [G4, F5, E5, D5:3];
      play [F4, E5, D5, C5:3];
      play [E4, D5, C5, B4:3];
   }

   pattern main() {
      for (let i = 0; i < 2; i = i + 1) {
         play main_phrase([E4, G#4, B4, C5:3, E4]);
         play main_phrase([E4, C5, B4, A4:(i == 1 ? 3 : 4)]);
      }

      play followup_phrase();

      play alternate(E4, E5, 4);
      play [E5, E6];
      play alternate(D#5, E5, 7);
   }

   loop (2) play main();
}

/* To be played with the left hand. */
track backing using piano {
   pattern phrase(rest_time_last) { 
      play [A2, E3, A3, rest:3];
      play [E2, E3, G#3, rest:3];
      play [A2, E3, A3, rest:rest_time_last];
   }

   pattern next_phrase() {
      play [C3, G3, C4, rest:3];
      play [G2, G3, B3, rest:3];
      play [A2, E3, A3, rest:3];
      play [E2, E3, E4, rest:3];
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
