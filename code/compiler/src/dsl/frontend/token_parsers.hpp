#pragma once

#include <concepts>
#include <stdexcept>
#include <string>

#include "dsl/errors/lexical_error.hpp"
#include "dsl/music/note.hpp"
#include "parser.hpp"

namespace dsl::frontend {

using errors::LexicalError;

music::Note parse_note_literal(const char* yytext, const int yyleng);

template <typename T>
    requires std::same_as<T, int> || std::same_as<T, double>
T parse_numeric_literal(const char* yytext, const int yyleng, const Parser::location_type& loc) {
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

}  // namespace dsl::frontend
