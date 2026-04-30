#pragma once

namespace dsl::ir::detail {

struct NoteValue {
    int midi_note{};
    double duration_beats{1.0};
    int velocity{100};
};

}  // namespace dsl::ir::detail