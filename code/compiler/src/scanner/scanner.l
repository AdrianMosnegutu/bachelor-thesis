%option c++
%option noyywrap
%option nodefault
%option yylineno
%option outfile="scanner_lex.cpp"
%option header-file="scanner_lex.hpp"

%{
#include "scanner/scanner_runtime.hpp"

#define YY_USER_ACTION                                                                      \
    {                                                                                       \
        ::compiler::scanner::internal::runtime().update_token_span(std::string_view{        \
            YYText(),                                                                       \
            static_cast<std::size_t>(YYLeng())                                              \
        }, static_cast<std::size_t>(lineno()));                                             \
    }
%}

WHITESPACE   [ \t\r\n]+
COMMENT      ^[ \t]*#[^\n]*
VARIABLE     \$[A-Za-z_][A-Za-z0-9_]*
IDENTIFIER   [A-Za-z_][A-Za-z0-9_]*
INVALID_NOTE (B#|Cb|E#|Fb)([0-9]+)?
NOTE         ([A-G][0-9]+)|(([ACDFG]#|[ABDEG]b)[0-9]+)|([ACDFG]#|[ABDEG]b)
NUMBER       [0-9]+(\.[0-9]+)?

%%

{WHITESPACE}           ;
{COMMENT}              ;

"tempo"                { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordTempo, YYText()); }
"time_signature"       { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordTimeSignature, YYText()); }
"key"                  { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordKey, YYText()); }
"pattern"              { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordPattern, YYText()); }
"play"                 { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordPlay, YYText()); }
"from"                 { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordFrom, YYText()); }
"loop"                 { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordLoop, YYText()); }
"num"                  { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordNum, YYText()); }
"note"                 { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordNote, YYText()); }
"chord"                { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordChord, YYText()); }
"if"                   { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordIf, YYText()); }
"else"                 { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordElse, YYText()); }
"using"                { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::KeywordUsing, YYText()); }

";"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::SeparatorSemicolon, YYText()); }
","                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::SeparatorComma, YYText()); }
"("                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::SeparatorLeftParenthesis, YYText()); }
")"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::SeparatorRightParenthesis, YYText()); }
"{"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::SeparatorLeftBrace, YYText()); }
"}"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::SeparatorRightBrace, YYText()); }

"="                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::OperatorAssign, YYText()); }
"+"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::OperatorPlus, YYText()); }
"-"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::OperatorMinus, YYText()); }
"*"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::OperatorMultiply, YYText()); }
"/"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::OperatorDivide, YYText()); }
"%"                    { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::OperatorModulo, YYText()); }

{INVALID_NOTE}         { return ::compiler::scanner::internal::make_error_token(YYText(), "Invalid note accidental."); }
{NOTE}                 { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::ConstantNote, YYText()); }
{NUMBER}               { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::ConstantNumber, YYText()); }

{VARIABLE}             { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::Identifier, YYText()); }
{IDENTIFIER}           { return ::compiler::scanner::internal::make_parser_token(::compiler::scanner::TokenType::Identifier, YYText()); }

<<EOF>>                { return 0; }

.                      { return ::compiler::scanner::internal::make_error_token(YYText(), "Unexpected character."); }

%%
