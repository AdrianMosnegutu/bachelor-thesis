#pragma once

#include <string>
#include <string_view>

#include "scanner/source_location.hpp"

namespace compiler::scanner {

/**
 * @brief Token codes returned by the scanner for parser consumption.
 */
enum class TokenType {
    Eof = 0,

    Identifier,

    KeywordTempo,
    KeywordTimeSignature,
    KeywordKey,
    KeywordPattern,
    KeywordPlay,
    KeywordFrom,
    KeywordLoop,
    KeywordNum,
    KeywordNote,
    KeywordChord,
    KeywordIf,
    KeywordElse,
    KeywordUsing,

    ConstantNote,
    ConstantNumber,

    SeparatorSemicolon,
    SeparatorComma,
    SeparatorLeftParenthesis,
    SeparatorRightParenthesis,
    SeparatorLeftBrace,
    SeparatorRightBrace,

    OperatorAssign,
    OperatorPlus,
    OperatorMinus,
    OperatorMultiply,
    OperatorDivide,
    OperatorModulo,

    Error = 256,
};

/**
 * @brief Semantic value produced for the current token.
 */
struct SemanticValue {
    std::string_view lexeme{};
};

/**
 * @brief Token produced by the scanner together with source information.
 */
struct ScannedToken {
    TokenType kind{TokenType::Eof};
    SourceSpan span{};
    std::string lexeme{};
};

}  // namespace compiler::scanner
