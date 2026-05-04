tempo 140;
signature 4/4;

pattern rhythm(n) {
    loop (n) {
        play A4;
        play @;
        play B4;
    }
}

pattern melody() {
    play [C5, E5, G5];
    play $ D5;
    play rest;
}

track percussion {
    let rep = 2;
    play rhythm(rep);
    play melody();
}
