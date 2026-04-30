#include "dsl/errors/semantic_error.hpp"

#include <sstream>

#include "dsl/errors/terminal_color.hpp"

namespace dsl::errors {

using detail::TerminalColor;

std::string SemanticError::format() const {
    std::stringstream ss;

    ss << TerminalColor::BoldRed << "semantic error: " << TerminalColor::Reset << " ";
    ss << loc_str_ << ": " << msg_;

    return ss.str();
}

}  // namespace dsl::errors
