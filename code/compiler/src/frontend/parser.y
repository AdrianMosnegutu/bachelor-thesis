%language "c++"
%require  "3.8"
%locations

%define api.namespace    {dsl::frontend}
%define api.parser.class {Parser}
%define api.token.prefix {TOK_}
%define api.value.type variant
%define api.token.constructor
%define api.location.file "bison_location.hpp"
%define api.value.automove
%define parse.error detailed
%define parse.lac full
%expect 1

// -- Code sections ----------------------------------------------------------------------------------------------------

%code requires {
    #include <memory>
    #include <optional>
    #include <string>
    #include <utility>
    #include <variant>
    #include <vector>

    #include "dsl/core/ast/program.hpp"
    #include "dsl/core/errors/syntax_error.hpp"
    #include "dsl/core/music/drum_note.hpp"
    #include "dsl/core/music/instrument.hpp"
    #include "dsl/core/music/note.hpp"
    #include "dsl/core/music/pitch.hpp"
    #include "dsl/core/location.hpp"
}

%code provides {
    dsl::frontend::Parser::symbol_type yylex(dsl::frontend::Parser::location_type& loc);
}

%code {
    namespace dsl::frontend {
    namespace {

    namespace music = dsl::music;
    namespace ast = dsl::ast;

    using dsl::errors::SyntaxError;

    using ast::BinaryExpression;
    using ast::BinaryOperator;
    using ast::Expression;
    using ast::ExpressionPtr;
    using ast::ExpressionKind;

    ExpressionPtr make_expr(ExpressionKind k, dsl::Location loc) {
        return std::make_unique<Expression>(Expression{std::move(k), std::move(loc)});
    }

    ExpressionPtr bin_op(
        BinaryOperator op, ExpressionPtr lhs, ExpressionPtr rhs, dsl::Location loc) {
        return make_expr(BinaryExpression{op, std::move(lhs), std::move(rhs)}, std::move(loc));
    }

    }  // namespace
    }  // namespace dsl::frontend
}

// -- Parameters -------------------------------------------------------------------------------------------------------

%param       { dsl::frontend::Parser::location_type& loc }
%parse-param { dsl::ast::Program& program_out }

// -- Keyword Tokens ---------------------------------------------------------------------------------------------------

%token                        TEMPO               "tempo"
%token                        SIGNATURE           "signature"
%token                        KEY                 "key"
%token                        TRACK               "track"
%token                        PATTERN             "pattern"
%token                        PLAY                "play"
%token                        FOR                 "for"
%token                        LOOP                "loop"
%token                        IF                  "if"
%token                        ELSE                "else"
%token                        LET                 "let"
%token                        USING               "using"
%token                        FROM                "from"
%token                        VOICE               "voice"
%token                        REST                "rest"

// -- Typed Tokens -----------------------------------------------------------------------------------------------------

%token <double>               FLOAT_LIT           "float"
%token <int>                  INT_LIT             "integer"
%token <bool>                 BOOL_LIT            "boolean"
%token <music::Instrument>    INSTRUMENT_LIT      "instrument"
%token <music::DrumNote>      DRUM_NOTE_LIT       "drum_note"
%token <music::Note>          NOTE_LIT            "note"
%token <std::string>          IDENT               "identifier"

// -- Arithmetic Operator Tokens ---------------------------------------------------------------------------------------

%token                        PLUS                "+"
%token                        MINUS               "-"
%token                        STAR                "*"
%token                        SLASH               "/"
%token                        PERCENT             "%"

// -- Comparison Operator Tokens ---------------------------------------------------------------------------------------

%token                        EQ                  "="
%token                        EQEQ                "=="
%token                        NEQ                 "!="
%token                        LT                  "<"
%token                        GT                  ">"
%token                        LEQ                 "<="
%token                        GEQ                 ">="

// -- Logical Operator Tokens ------------------------------------------------------------------------------------------

%token                        AND                 "&&"
%token                        OR                  "||"
%token                        NOT                 "!"

// -- Ternary Operator Token -------------------------------------------------------------------------------------------

%token                        QUESTION            "?"

// -- Separator Tokens -------------------------------------------------------------------------------------------------

%token                        SEMICOLON           ";"
%token                        COMMA               ","
%token                        COLON               ":"
%token                        LPAREN              "("
%token                        RPAREN              ")"
%token                        LBRACKET            "["
%token                        RBRACKET            "]"
%token                        LBRACE              "{"
%token                        RBRACE              "}"

// -- Non-terminal types -----------------------------------------------------------------------------------------------

%type <ast::TempoDeclaration>                 tempo_decl
%type <ast::SignatureDeclaration>             signature_decl
%type <ast::PatternDefinition>                pattern_def
%type <ast::TrackDefinition>                  track_decl
%type <std::optional<std::string>>            opt_track_name
%type <std::optional<music::Instrument>>      opt_using
%type <std::vector<ast::TrackItem>>           track_body
%type <ast::VoiceDefinition>                  voice_decl
%type <std::vector<ast::VoiceItem>>           voice_body
%type <std::vector<std::string>>              opt_param_list param_list
%type <ast::Block>                            block stmt_list
%type <ast::StatementPtr>                     stmt let_stmt assign_stmt
%type <ast::StatementPtr>                     play_stmt for_stmt if_stmt loop_stmt
%type <ast::StatementPtr>                     let_decl assignment
%type <ast::StatementPtr>                     for_init for_step
%type <ast::ExpressionPtr>                    for_cond
%type <ast::PlayTarget>                       play_target
%type <ast::PlaySource>                       durational_source plain_source
%type <ast::ExpressionPtr>                    ident_play_source
%type <ast::ExpressionPtr>                    opt_duration opt_from
%type <ast::ExpressionPtr>                    chord sequence
%type <std::vector<ast::ExpressionPtr>>       expr_list opt_arg_list
%type <std::vector<ast::SequenceItem>>        sequence_items chord_items
%type <ast::SequenceItem>                     sequence_item chord_item
%type <ast::ExpressionPtr>                    expr ternary_expr or_expr and_expr eq_expr rel_expr
%type <ast::ExpressionPtr>                    add_expr mul_expr unary_expr primary
%type <ast::Block>                            body
%type <ast::StatementPtr>                     unbrace_stmt

%%

// -- Program ----------------------------------------------------------------------------------------------------------

program
    : header top_items
      { }
    ;

// -- Header -----------------------------------------------------------------------------------------------------------

// Each declaration may appear at most once, in any order, before any globals
// or tracks. Duplicates are rejected at parse time.
header
    : %empty
      { }
    | header tempo_decl
      {
          if (program_out.header.tempo.has_value()) {
              throw SyntaxError(@2, "duplicate 'tempo' declaration");
          }
          program_out.header.tempo = $2;
      }
    | header signature_decl
      {
          if (program_out.header.signature.has_value()) {
              throw SyntaxError(@2, "duplicate 'signature' declaration");
          }
          program_out.header.signature = $2;
      }
    ;

tempo_decl
    : "tempo" "integer" ";"
      { $$ = ast::TempoDeclaration{$2, @$}; }
    ;

signature_decl
    : "signature" "integer" "/" "integer" ";"
      { $$ = ast::SignatureDeclaration{$2, $4, @$}; }
    ;

// -- Top-level items --------------------------------------------------------------------------------------------------

top_items
    : %empty
      { }
    | top_items top_item
      { }
    ;

top_item
    : let_stmt
      { program_out.globals.emplace_back($1); }
    | pattern_def
      { program_out.globals.emplace_back($1); }
    | track_decl
      { program_out.tracks.push_back($1); }
    ;

// -- Tracks -----------------------------------------------------------------------------------------------------------

track_decl
    : "track" opt_track_name opt_using "{" track_body "}"
      { $$ = ast::TrackDefinition{$2, $3, $5, @$}; }
    ;

opt_track_name
    : %empty
      { $$ = std::nullopt; }
    | "identifier"
      { $$ = $1; }
    ;

opt_using
    : %empty
      { $$ = std::nullopt; }
    | "using" "instrument"
      { $$ = $2; }
    ;

track_body
    : %empty
      { }
    | track_body stmt
      { $$ = $1; $$.emplace_back($2); }
    | track_body pattern_def
      { $$ = $1; $$.emplace_back($2); }
    | track_body voice_decl
      { $$ = $1; $$.emplace_back($2); }
    ;

// -- Voices -----------------------------------------------------------------------------------------------------------

// voice_decl is only reachable from track_body, so nesting is impossible at
// the grammar level (voice_body does not include voice_decl).
voice_decl
    : "voice" "{" voice_body "}"
      { $$ = ast::VoiceDefinition{std::nullopt, $3, @$}; }
    | "voice" "from" expr "{" voice_body "}"
      { $$ = ast::VoiceDefinition{std::optional<ast::ExpressionPtr>{std::move($3)}, $5, @$}; }
    ;

voice_body
    : %empty
      { }
    | voice_body stmt
      { $$ = $1; $$.emplace_back($2); }
    | voice_body pattern_def
      { $$ = $1; $$.emplace_back($2); }
    ;

// -- Patterns ---------------------------------------------------------------------------------------------------------

// Pattern bodies use the plain `block` non-terminal, so nested `pattern`
// definitions are syntactically impossible inside a pattern.
pattern_def
    : "pattern" "identifier" "(" opt_param_list ")" block
      { $$ = ast::PatternDefinition{$2, $4, $6, @$}; }
    ;

opt_param_list
    : %empty
      { }
    | param_list
      { $$ = $1; }
    ;

param_list
    : "identifier"
      { $$.push_back($1); }
    | param_list "," "identifier"
      { $$ = $1; $$.push_back($3); }
    ;

// -- Blocks & statements ----------------------------------------------------------------------------------------------

block
    : "{" stmt_list "}"
      { $$ = $2; }
    ;

body
    : block
      { $$ = std::move($1); }
    | unbrace_stmt
      { $$ = ast::Block{}; $$.push_back(std::move($1)); }
    ;

unbrace_stmt
    : assign_stmt
      { $$ = $1; }
    | play_stmt
      { $$ = $1; }
    | for_stmt
      { $$ = $1; }
    | loop_stmt
      { $$ = $1; }
    | if_stmt
      { $$ = $1; }
    ;

stmt_list
    : %empty
      { }
    | stmt_list stmt
      { $$ = $1; $$.push_back($2); }
    ;

stmt
    : let_stmt
      { $$ = $1; }
    | assign_stmt
      { $$ = $1; }
    | play_stmt
      { $$ = $1; }
    | for_stmt
      { $$ = $1; }
    | loop_stmt
      { $$ = $1; }
    | if_stmt
      { $$ = $1; }
    ;

let_stmt
    : let_decl ";"
      { $$ = $1; }
    ;

assign_stmt
    : assignment ";"
      { $$ = $1; }
    ;

let_decl
    : "let" "identifier" "=" expr
      { $$ = std::make_unique<ast::Statement>(ast::Statement{ast::LetStatement{$2, $4}, @$}); }
    ;

assignment
    : "identifier" "=" expr
      { $$ = std::make_unique<ast::Statement>(ast::Statement{ast::AssignStatement{$1, $3}, @$}); }
    ;

for_stmt
    : "for" "(" for_init ";" for_cond ";" for_step ")" body
      { $$ = std::make_unique<ast::Statement>(ast::Statement{ast::ForStatement{$3, $5, $7, $9}, @$}); }
    ;

for_init
    : %empty
      { $$ = nullptr; }
    | let_decl
      { $$ = $1; }
    | assignment
      { $$ = $1; }
    ;

for_cond
    : %empty
      { $$ = nullptr; }
    | expr
      { $$ = $1; }
    ;

for_step
    : %empty
      { $$ = nullptr; }
    | assignment
      { $$ = $1; }
    ;

loop_stmt
    : "loop" "(" expr ")" body
      { $$ = std::make_unique<ast::Statement>(ast::Statement{ast::LoopStatement{$3, $5}, @$}); }
    ;

if_stmt
    : "if" "(" expr ")" body
      { $$ = std::make_unique<ast::Statement>(ast::Statement{ast::IfStatement{$3, $5}, @$}); }
    | "if" "(" expr ")" body "else" body
      { $$ = std::make_unique<ast::Statement>(ast::Statement{ast::IfStatement{$3, $5, std::optional<ast::Block>{std::move($7)}}, @$}); }
    ;

// -- Play -------------------------------------------------------------------------------------------------------------

play_stmt
    : "play" play_target ";"
      { $$ = std::make_unique<ast::Statement>(ast::Statement{ast::PlayStatement{$2}, @$}); }
    ;

play_target
    : durational_source opt_duration opt_from
      { $$ = ast::PlayTarget{$1, $2, $3, @$}; }
    | plain_source opt_from
      { $$ = ast::PlayTarget{$1, nullptr, $2, @$}; }
    ;

durational_source
    : "note"
      { $$ = make_expr(ast::NoteLiteral{$1}, @$); }
    | "rest"
      { $$ = make_expr(ast::RestLiteral{}, @$); }
    | chord
      { $$ = $1; }
    | ident_play_source
      { $$ = $1; }
    | "(" expr ")"
      { $$ = make_expr(ast::ParenthesisedExpression{$2}, @$); }
    ;

plain_source
    : sequence
      { $$ = $1; }
    | "drum_note"
      { $$ = $1; }
    ;

ident_play_source
    : "identifier"
      { $$ = make_expr(ast::IdentifierExpression{$1}, @$); }
    | "identifier" "(" opt_arg_list ")"
      { $$ = make_expr(ast::CallExpression{$1, $3}, @$); }
    ;

opt_duration
    : %empty
      { $$ = nullptr; }
    | ":" expr
      { $$ = $2; }
    ;

opt_from
    : %empty
      { $$ = nullptr; }
    | "from" expr
      { $$ = $2; }
    ;

// -- Sequences & chords -----------------------------------------------------------------------------------------------

chord
    : "(" chord_item "," chord_items ")"
      {
          auto notes = $4;
          notes.insert(notes.begin(), $2);
          $$ = make_expr(ast::ChordExpression{std::move(notes)}, @$);
      }
    ;

chord_items
    : chord_item
      { $$.push_back($1); }
    | chord_items "," chord_item
      { $$ = $1; $$.push_back($3); }
    ;

// `rest` cannot appear inside a chord — only pitched expressions are valid.
chord_item
    : expr opt_duration
      { $$ = ast::SequenceItem{$1, $2}; }
    ;

sequence
    : "[" sequence_items "]"
      { $$ = make_expr(ast::SequenceExpression{$2}, @$); }
    ;

sequence_items
    : sequence_item
      { $$.push_back($1); }
    | sequence_items "," sequence_item
      { $$ = $1; $$.push_back($3); }
    ;

sequence_item
    : expr opt_duration
      { $$ = ast::SequenceItem{$1, $2}; }
    | "rest" opt_duration
      { $$ = ast::SequenceItem{make_expr(ast::RestLiteral{}, @1), $2}; }
    ;

opt_arg_list
    : %empty
      { }
    | expr_list
      { $$ = $1; }
    ;

expr_list
    : expr
      { $$.push_back($1); }
    | expr_list "," expr
      { $$ = $1; $$.push_back($3); }
    ;

// -- Expressions ------------------------------------------------------------------------------------------------------

// `rest` and pattern calls are intentionally absent: `rest` is only permitted
// in play/sequence position; calls only in play position.
expr
    : ternary_expr
      { $$ = $1; }
    ;

ternary_expr
    : or_expr
      { $$ = $1; }
    | or_expr "?" ternary_expr ":" ternary_expr
      { $$ = make_expr(ast::TernaryExpression{std::move($1), std::move($3), std::move($5)}, @$); }
    ;


or_expr
    : and_expr
      { $$ = $1; }
    | or_expr "||" and_expr
      { $$ = bin_op(ast::BinaryOperator::Or,  $1, $3, @$); }
    ;

and_expr
    : eq_expr
      { $$ = $1; }
    | and_expr "&&" eq_expr
      { $$ = bin_op(ast::BinaryOperator::And, $1, $3, @$); }
    ;

eq_expr
    : rel_expr
      { $$ = $1; }
    | eq_expr "==" rel_expr
      { $$ = bin_op(ast::BinaryOperator::Equals,    $1, $3, @$); }
    | eq_expr "!=" rel_expr
      { $$ = bin_op(ast::BinaryOperator::NotEquals, $1, $3, @$); }
    ;

rel_expr
    : add_expr
      { $$ = $1; }
    | rel_expr "<"  add_expr
      { $$ = bin_op(ast::BinaryOperator::Less,         $1, $3, @$); }
    | rel_expr ">"  add_expr
      { $$ = bin_op(ast::BinaryOperator::Greater,      $1, $3, @$); }
    | rel_expr "<=" add_expr
      { $$ = bin_op(ast::BinaryOperator::LessOrEqual,  $1, $3, @$); }
    | rel_expr ">=" add_expr
      { $$ = bin_op(ast::BinaryOperator::GreaterOrEqual, $1, $3, @$); }
    ;

add_expr
    : mul_expr
      { $$ = $1; }
    | add_expr "+" mul_expr
      { $$ = bin_op(ast::BinaryOperator::Add,      $1, $3, @$); }
    | add_expr "-" mul_expr
      { $$ = bin_op(ast::BinaryOperator::Subtract, $1, $3, @$); }
    ;

mul_expr
    : unary_expr
      { $$ = $1; }
    | mul_expr "*" unary_expr
      { $$ = bin_op(ast::BinaryOperator::Multiply, $1, $3, @$); }
    | mul_expr "/" unary_expr
      { $$ = bin_op(ast::BinaryOperator::Divide,   $1, $3, @$); }
    | mul_expr "%" unary_expr
      { $$ = bin_op(ast::BinaryOperator::Modulo,   $1, $3, @$); }
    ;

unary_expr
    : primary
      { $$ = $1; }
    | "-" unary_expr
      { $$ = make_expr(ast::UnaryExpression{ast::UnaryOperator::Negative, $2}, @$); }
    | "!" unary_expr
      { $$ = make_expr(ast::UnaryExpression{ast::UnaryOperator::Not,      $2}, @$); }
    ;

primary
    : "integer"
      { $$ = make_expr(ast::IntLiteral{$1}, @$); }
    | "float"
      { $$ = make_expr(ast::FloatLiteral{$1}, @$); }
    | "boolean"
      { $$ = make_expr(ast::BoolLiteral{$1}, @$); }
    | "note"
      { $$ = make_expr(ast::NoteLiteral{$1}, @$); }
    | "identifier"
      { $$ = make_expr(ast::IdentifierExpression{$1}, @$); }
    | sequence
      { $$ = $1; }
    | chord
      { $$ = $1; }
    | "(" expr ")"
      { $$ = make_expr(ast::ParenthesisedExpression{$2}, @$); }
    ;

%%

// -- Epilogue ---------------------------------------------------------------------------------------------------------

namespace dsl::frontend {

void Parser::error(const location_type& loc, const std::string& msg) {
    throw dsl::errors::SyntaxError(loc, msg);
}

}  // namespace dsl::frontend
