tempo 100;
signature 3/4;

pattern phrase(dur, count) {
    loop (count) {
        play A4 :dur;
        play C5 :dur;
    }
}

track {
    let beats = 2;
    play phrase(beats, 4);
    play undefined_melody;
    play missing_note :beats;
    let volume = ghost_variable + 1;
    play A4 :volume;
    play B4;
}
