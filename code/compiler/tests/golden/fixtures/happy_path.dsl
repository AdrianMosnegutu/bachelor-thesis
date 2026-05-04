tempo 120;
signature 4/4;

pattern motif(dur) {
    play A4 :dur;
    play rest :dur;
    play C5 :dur;
}

pattern bass_line(rep) {
    let step = rep > 1 ? 2 : 1;
    loop (rep) {
        play A2 :step;
        play E3 :step;
    }
}

track melody using piano {
    let q = 1;
    let h = 2;
    play motif(q);
    play motif(h);
    play (E4, G4, B4);
    loop (2) {
        play A4;
    }
}

track bassline using bass {
    play bass_line(2);
    play A2 :4;
}
