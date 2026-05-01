%option noyywrap nounput noinput batch
%option never-interactive

/* -- Code section -------------------------------------------------------------------------------------------------- */

%{
#include "dsl/diagnostics/diagnostics_engine.hpp"
#include "dsl/frontend/detail/scanner_helpers.hpp"
#include "dsl/music/drum_note.hpp"
#include "dsl/music/instrument.hpp"
#include "parser.hpp"

#undef  YY_DECL
#define YY_DECL P::symbol_type yylex(P::location_type& loc)

// Advance the token's source location by the number of characters matched.
#define YY_USER_ACTION loc.columns(yyleng);

namespace {

using namespace dsl::frontend::detail;

using dsl::music::DrumNote;
using dsl::music::Instrument;
using P = dsl::frontend::Parser;

dsl::DiagnosticsEngine* current_diagnostics = nullptr;

void report_lexical(const P::location_type& loc, const std::string& message) {
    if (current_diagnostics != nullptr) {
        current_diagnostics->report(dsl::DiagnosticStage::Lexical, dsl::DiagnosticSeverity::Error, loc, message);
        return;
    }

    throw lexical_error(loc, message);
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
    <<EOF>>                     { report_lexical(loc, "unterminated block comment"); return P::make_YYEOF(loc); }
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

"piano"                         { return P::make_INSTRUMENT_LIT(Instrument::Piano,  loc); }
"guitar"                        { return P::make_INSTRUMENT_LIT(Instrument::Guitar, loc); }
"bass"                          { return P::make_INSTRUMENT_LIT(Instrument::Bass,   loc); }
"violin"                        { return P::make_INSTRUMENT_LIT(Instrument::Violin, loc); }
"drums"                         { return P::make_INSTRUMENT_LIT(Instrument::Drums,  loc); }

 /* -- Drum note keywords ------------------------------------------------------------------------------------------- */

"kick"                          { return P::make_DRUM_NOTE_LIT(DrumNote::Kick,  loc); }
"snare"                         { return P::make_DRUM_NOTE_LIT(DrumNote::Snare, loc); }
"hihat"                         { return P::make_DRUM_NOTE_LIT(DrumNote::Hihat, loc); }
"crash"                         { return P::make_DRUM_NOTE_LIT(DrumNote::Crash, loc); }
"ride"                          { return P::make_DRUM_NOTE_LIT(DrumNote::Ride,  loc); }

 /* -- Notes  ------------------------------------------------------------------------------------------------------- */

{pitch}{accidental}?{octave}    { return P::make_NOTE_LIT(parse_note_literal(yytext, yyleng), loc); }

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

.                               { report_lexical(loc, std::string("unexpected character: '") + yytext + "'"); }

%%

// -- Epilogue ---------------------------------------------------------------------------------------------------------

void scanner_set_diagnostics(dsl::DiagnosticsEngine* diagnostics) {
    current_diagnostics = diagnostics;
}

// Reset scanner to INITIAL state between test cases.
void scanner_reset() {
    BEGIN(INITIAL);
    yylineno = 1;
}
