#pragma once

#include "dsl/ast/expressions/sequence_expression.hpp"

namespace dsl::ast {

struct ChordExpression {
    std::vector<SequenceItem> notes;
};

}  // namespace dsl::ast
