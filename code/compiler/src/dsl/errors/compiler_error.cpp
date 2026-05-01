#include "dsl/errors/compiler_error.hpp"

#include <sstream>

namespace dsl::errors {

CompilerError::CompilerError(const source::Location& loc, const std::string& msg)
    : std::runtime_error(loc_to_string(loc) + ": " + msg), loc_str_(loc_to_string(loc)), msg_(msg) {}

std::string CompilerError::loc_to_string(const source::Location& loc) {
    std::ostringstream oss;
    oss << loc;
    return oss.str();
}

}  // namespace dsl::errors
