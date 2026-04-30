#include "dsl/errors/syntax_error.hpp"

#include <sstream>

#include "dsl/errors/terminal_color.hpp"

namespace dsl::errors {

using detail::TerminalColor;

std::string SyntaxError::format() const {
    std::stringstream ss;

    ss << TerminalColor::BoldRed << "syntax error: " << TerminalColor::Reset << " ";
    ss << loc_str_ << ": " << msg_;

    return ss.str();
}

}  // namespace dsl::errors
