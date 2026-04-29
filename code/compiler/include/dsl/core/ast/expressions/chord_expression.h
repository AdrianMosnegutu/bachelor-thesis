#pragma once

#include "dsl/core/ast/expressions/sequence_expression.h"

namespace dsl::ast {

struct ChordExpression {
    std::vector<SequenceItem> notes;
};

}  // namespace dsl::ast
