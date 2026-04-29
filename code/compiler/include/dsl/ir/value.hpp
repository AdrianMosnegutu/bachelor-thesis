#pragma once

#include <variant>

#include "values/chord_value.h"
#include "values/note_value.h"
#include "values/rest_value.h"
#include "values/sequence_value.h"

namespace dsl::ir {

using ValueKind = std::variant<int, double, bool, NoteValue, RestValue, SequenceValue, ChordValue>;

struct Value {
    ValueKind kind;
};

}  // namespace dsl::ir
