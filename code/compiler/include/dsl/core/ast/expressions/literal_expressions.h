#pragma once

#include "dsl/core/music/note.hpp"

namespace dsl::ast {

struct IntLiteralExpression {
    int value;
};

struct FloatLiteralExpression {
    double value;
};

struct BoolLiteralExpression {
    bool value;
};

struct NoteLiteralExpression {
    music::Note value;
};

struct RestLiteralExpression {};

}  // namespace dsl::ast
