#include "dsl/errors/lexical_error.hpp"

#include <sstream>

#include "dsl/errors/terminal_color.hpp"

namespace dsl::errors {

using detail::TerminalColor;

std::string LexicalError::format() const {
    std::stringstream ss;

    ss << TerminalColor::BoldRed << "lexical error:" << TerminalColor::Reset << " ";
    ss << loc_str_ << ": " << msg_;

    return ss.str();
}

}  // namespace dsl::errors
