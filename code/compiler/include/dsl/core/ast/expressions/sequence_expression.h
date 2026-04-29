#pragma once

#include <memory>
#include <vector>

namespace dsl::ast {

struct Expression;

struct SequenceItem {
    std::unique_ptr<Expression> value;
    std::unique_ptr<Expression> duration;
};

struct SequenceExpression {
    std::vector<SequenceItem> items;
};

}  // namespace dsl::ast
