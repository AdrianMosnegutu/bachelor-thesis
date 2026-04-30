#include "token_parsers.hpp"

namespace dsl::frontend {

namespace {

using music::Accidental;
using music::Note;
using music::Pitch;

}  // namespace

Note parse_note_literal(const char* yytext, const int yyleng) {
    const Pitch pitch = music::letter_to_pitch(yytext[0]);
    const Accidental accidental = yyleng > 1 ? music::char_to_accidental(yytext[1]) : Accidental::Natural;
    const uint8_t octave = yytext[yyleng - 1] - '0';

    return {pitch, accidental, octave};
}

}  // namespace dsl::frontend
