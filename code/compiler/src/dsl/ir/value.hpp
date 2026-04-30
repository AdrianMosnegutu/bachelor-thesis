#pragma once

#include <variant>

#include "values/chord_value.hpp"
#include "values/note_value.hpp"
#include "values/rest_value.hpp"
#include "values/sequence_value.hpp"

namespace dsl::ir {

using ValueKind = std::variant<int, double, bool, NoteValue, RestValue, SequenceValue, ChordValue>;

struct Value {
    ValueKind kind;
};

}  // namespace dsl::ir
