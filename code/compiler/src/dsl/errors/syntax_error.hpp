#pragma once

#include "dsl/errors/compiler_error.hpp"

namespace dsl::errors {

struct SyntaxError final : CompilerError {
    using CompilerError::CompilerError;

    [[nodiscard]] std::string format() const override;
};

}  // namespace dsl::errors
