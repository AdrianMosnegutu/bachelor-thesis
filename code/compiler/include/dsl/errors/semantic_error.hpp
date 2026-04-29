#pragma once

#include <sstream>

#include "dsl/errors/compiler_error.hpp"
#include "dsl/errors/terminal_color.hpp"

namespace dsl::errors {

struct SemanticError final : CompilerError {
    using CompilerError::CompilerError;

    [[nodiscard]] std::string format() const override {
        std::stringstream ss;

        ss << TerminalColor::BoldRed << "semantic error: " << TerminalColor::Reset << " ";
        ss << loc_str_ << ": " << msg_;

        return ss.str();
    }
};

}  // namespace dsl::errors
