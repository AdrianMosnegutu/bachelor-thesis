#pragma once

#include <vector>

#include "note_value.h"

namespace dsl::ir {

struct ChordValue {
    std::vector<NoteValue> notes;
    double duration_beats{1.0};
};

}  // namespace dsl::ir
