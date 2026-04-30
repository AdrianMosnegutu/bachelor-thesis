#pragma once

#include <variant>

#include "dsl/ast/expression.hpp"
#include "dsl/music/drum_note.hpp"

namespace dsl::ast {

using PlaySource = std::variant<ExpressionPtr, music::DrumNote>;

struct PlayTarget {
    PlaySource source;
    ExpressionPtr duration;     // null when absent
    ExpressionPtr from_offset;  // null when absent
    Location location;
};

struct PlayStatement {
    PlayTarget target;
};

}  // namespace dsl::ast
