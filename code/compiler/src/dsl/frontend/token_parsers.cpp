#include "token_parsers.hpp"

namespace dsl::frontend {

music::Note parse_note_literal(const char* yytext, const int yyleng) {
    music::Pitch pitch = music::letter_to_pitch(yytext[0]);
    music::Accidental accidental = (yyleng > 1) ? music::char_to_accidental(yytext[1]) : music::Accidental::Natural;
    uint8_t octave = yytext[yyleng - 1] - '0';

    return {pitch, accidental, octave};
}

}  // namespace dsl::frontend
