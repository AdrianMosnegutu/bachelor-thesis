#pragma once

#include <sstream>

#include "dsl/core/errors/compiler_error.hpp"
#include "dsl/core/errors/terminal_color.hpp"

namespace dsl::errors {

struct LexicalError final : CompilerError {
    using CompilerError::CompilerError;

    [[nodiscard]] std::string format() const override {
        std::stringstream ss;

        ss << TerminalColor::BoldRed << "lexical error:" << TerminalColor::Reset << " ";
        ss << loc_str_ << ": " << msg_;

        return ss.str();
    }
};

}  // namespace dsl::errors
