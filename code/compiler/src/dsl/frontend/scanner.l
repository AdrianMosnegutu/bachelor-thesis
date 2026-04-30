%option noyywrap nounput noinput batch
%option never-interactive

/* -- Code section -------------------------------------------------------------------------------------------------- */

%{
#include <cstdint>
#include <string>

#include "dsl/errors/lexical_error.hpp"
#include "dsl/music/drum_note.hpp"
#include "dsl/music/instrument.hpp"
#include "dsl/music/note.hpp"
#include "dsl/music/pitch.hpp"
#include "parser.hpp"

#undef  YY_DECL
#define YY_DECL P::symbol_type yylex(P::location_type& loc)

// Advance the token's source location by the number of characters matched.
#define YY_USER_ACTION loc.columns(yyleng);

namespace {

namespace music = dsl::music;

using P = dsl::frontend::Parser;
using dsl::errors::LexicalError;

music::Note parse_note(const char* yytext, const int yyleng) {
    music::Pitch pitch = music::letter_to_pitch(yytext[0]);
    music::Accidental accidental = (yyleng > 1) ? music::char_to_accidental(yytext[1]) : music::Accidental::Natural;
    uint8_t octave = yytext[yyleng - 1] - '0';

    return {pitch, accidental, octave};
}

template <typename T>
    requires std::same_as<T, int> || std::same_as<T, double>
T parse_numeric_literal(const char* yytext, const int yyleng, const P::location_type& loc) {
    const std::string literal_kind = std::same_as<T, int> ? "integer literal" : "floating point literal";
    const std::string lexeme(yytext, static_cast<std::size_t>(yyleng));

    try {
        std::size_t consumed_chars = 0;
        const T value = std::same_as<T, int> ? std::stoi(lexeme, &consumed_chars) : std::stod(lexeme, &consumed_chars);

        if (consumed_chars != lexeme.size()) {
            throw LexicalError(loc, "invalid " + literal_kind + ": '" + lexeme + "'");
        }

        return value;
    } catch (const std::invalid_argument&) {
        throw LexicalError(loc, "invalid " + literal_kind + ": '" + lexeme + "'");
    } catch (const std::out_of_range&) {
        throw LexicalError(loc, literal_kind + " out of range: '" + lexeme + "'");
    }
}

}  // namespace

%}

/* -- Start conditions ---------------------------------------------------------------------------------------------- */

%x BLOCK_COMMENT

/* -- Character class macros ---------------------------------------------------------------------------------------- */

digit        [0-9]
alpha        [A-Za-z_]
alnum        [A-Za-z0-9_]
pitch        [A-G]
accidental   [#b]
octave       [0-8]

%%

%{
    // Snap the token's begin position to its end so each token starts fresh.
    loc.step();
%}

 /* -- Whitespace & newlines ---------------------------------------------------------------------------------------- */

[ \t\r]+                        { loc.step(); }
\n+                             { loc.lines(yyleng); loc.step(); }

 /* -- Comments ----------------------------------------------------------------------------------------------------- */

"//"[^\n]*                      { loc.step(); }
"/*"                            { BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>{
    [^*\n]+                     { }
    \n+                         { loc.lines(yyleng); }
    "*"+[^*/\n]*                { }
    "*"+"/"                     { BEGIN(INITIAL); loc.step(); }
    <<EOF>>                     { throw LexicalError(loc, "unterminated block comment"); }
}

 /* -- Keywords ----------------------------------------------------------------------------------------------------- */

"tempo"                         { return P::make_TEMPO(loc); }
"signature"                     { return P::make_SIGNATURE(loc); }
"key"                           { return P::make_KEY(loc); }
"track"                         { return P::make_TRACK(loc); }
"pattern"                       { return P::make_PATTERN(loc); }
"play"                          { return P::make_PLAY(loc); }
"for"                           { return P::make_FOR(loc); }
"loop"                          { return P::make_LOOP(loc); }
"if"                            { return P::make_IF(loc); }
"else"                          { return P::make_ELSE(loc); }
"let"                           { return P::make_LET(loc); }
"using"                         { return P::make_USING(loc); }
"from"                          { return P::make_FROM(loc); }
"voice"                         { return P::make_VOICE(loc); }
"rest"                          { return P::make_REST(loc); }

 /* -- Numeric literals --------------------------------------------------------------------------------------------- */

{digit}+"."{digit}+             { return P::make_FLOAT_LIT(parse_numeric_literal<double>(yytext, yyleng, loc), loc); }
{digit}+                        { return P::make_INT_LIT(parse_numeric_literal<int>(yytext, yyleng, loc), loc); }

 /* -- Boolean literals --------------------------------------------------------------------------------------------- */

"true"                          { return P::make_BOOL_LIT(true,  loc); }
"false"                         { return P::make_BOOL_LIT(false, loc); }

 /* -- Instrument keywords ------------------------------------------------------------------------------------------ */

"piano"                         { return P::make_INSTRUMENT_LIT(music::Instrument::Piano,  loc); }
"guitar"                        { return P::make_INSTRUMENT_LIT(music::Instrument::Guitar, loc); }
"bass"                          { return P::make_INSTRUMENT_LIT(music::Instrument::Bass,   loc); }
"violin"                        { return P::make_INSTRUMENT_LIT(music::Instrument::Violin, loc); }
"drums"                         { return P::make_INSTRUMENT_LIT(music::Instrument::Drums,  loc); }

 /* -- Drum note keywords ------------------------------------------------------------------------------------------- */

"kick"                          { return P::make_DRUM_NOTE_LIT(music::DrumNote::Kick,  loc); }
"snare"                         { return P::make_DRUM_NOTE_LIT(music::DrumNote::Snare, loc); }
"hihat"                         { return P::make_DRUM_NOTE_LIT(music::DrumNote::Hihat, loc); }
"crash"                         { return P::make_DRUM_NOTE_LIT(music::DrumNote::Crash, loc); }
"ride"                          { return P::make_DRUM_NOTE_LIT(music::DrumNote::Ride,  loc); }

 /* -- Notes  ------------------------------------------------------------------------------------------------------- */

{pitch}{accidental}?{octave}    { return P::make_NOTE_LIT(parse_note(yytext, yyleng), loc); }

 /* -- Identifiers -------------------------------------------------------------------------------------------------- */

{alpha}{alnum}*                 { return P::make_IDENT(std::string(yytext, yyleng), loc); }

 /* -- Operators ---------------------------------------------------------------------------------------------------- */

"=="                            { return P::make_EQEQ(loc); }
"!="                            { return P::make_NEQ(loc); }
"<="                            { return P::make_LEQ(loc); }
">="                            { return P::make_GEQ(loc); }
"&&"                            { return P::make_AND(loc); }
"||"                            { return P::make_OR(loc); }

"+"                             { return P::make_PLUS(loc); }
"-"                             { return P::make_MINUS(loc); }
"*"                             { return P::make_STAR(loc); }
"/"                             { return P::make_SLASH(loc); }
"%"                             { return P::make_PERCENT(loc); }
"="                             { return P::make_EQ(loc); }
"<"                             { return P::make_LT(loc); }
">"                             { return P::make_GT(loc); }
"!"                             { return P::make_NOT(loc); }

 /* -- Punctuation -------------------------------------------------------------------------------------------------- */

"?"                             { return P::make_QUESTION(loc); }
";"                             { return P::make_SEMICOLON(loc); }
","                             { return P::make_COMMA(loc); }
":"                             { return P::make_COLON(loc); }
"("                             { return P::make_LPAREN(loc); }
")"                             { return P::make_RPAREN(loc); }
"["                             { return P::make_LBRACKET(loc); }
"]"                             { return P::make_RBRACKET(loc); }
"{"                             { return P::make_LBRACE(loc); }
"}"                             { return P::make_RBRACE(loc); }

 /* -- End-of-file & fallback --------------------------------------------------------------------------------------- */

<<EOF>>                         { return P::make_YYEOF(loc); }

.                               { throw LexicalError(loc, std::string("unexpected character: '") + yytext + "'"); }

%%

// -- Epilogue ---------------------------------------------------------------------------------------------------------

// Reset scanner to INITIAL state between test cases.
void scanner_reset() {
    BEGIN(INITIAL);
    yylineno = 1;
}
