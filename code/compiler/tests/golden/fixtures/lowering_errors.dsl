tempo 120;
signature 4/4;

track lead {
    let base = 8;
    let divisor = 0;
    let duration = base / divisor;
    play A4 :duration;
    play C5;
}

track harmony {
    let scale = 3;
    let shift = scale - 3;
    let length = scale / shift;
    play E4 :length;
    play G4;
}
