#pragma once

#include <variant>

#include "dsl/core/music/note.hpp"

namespace dsl::ast {

struct IntLiteral {
    int value;
};
struct FloatLiteral {
    double value;
};
struct BoolLiteral {
    bool value;
};
struct NoteLiteral {
    music::Note value;
};
struct RestLiteral {};

using LiteralKind = std::variant<IntLiteral, FloatLiteral, BoolLiteral, NoteLiteral, RestLiteral>;

struct LiteralExpression {
    LiteralKind value;
};

}  // namespace dsl::ast
