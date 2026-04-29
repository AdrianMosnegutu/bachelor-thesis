#pragma once

#include <optional>
#include <vector>

#include "dsl/core/ast/expression.hpp"
#include "pattern_definition.h"

namespace dsl::ast {

using VoiceItem = std::variant<StatementPtr, PatternDefinition>;

struct VoiceDefinition {
    std::optional<ExpressionPtr> from_expression;
    std::vector<VoiceItem> body;
    Location location;
};

}  // namespace dsl::ast
