#pragma once

#include <optional>

#include "dsl/semantic/symbol.hpp"
#include "dsl/semantic/type.hpp"

namespace dsl::semantic::detail {

struct ExpressionAnnotation {
    Type type;
    std::optional<SymbolId> resolved_symbol;
};

}  // namespace dsl::semantic::detail
