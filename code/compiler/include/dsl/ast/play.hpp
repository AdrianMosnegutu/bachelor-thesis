#pragma once

#include <variant>

#include "dsl/ast/expr.hpp"
#include "dsl/location.hpp"
#include "dsl/music/drum_note.hpp"

namespace dsl::ast {

using PlaySource = std::variant<ExpressionPtr, music::DrumNote>;

struct PlayTarget {
    PlaySource source;
    ExpressionPtr duration;     // null when absent
    ExpressionPtr from_offset;  // null when absent
    Location loc;
};

}  // namespace dsl::ast
