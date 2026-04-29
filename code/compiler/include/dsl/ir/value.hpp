#pragma once

#include <memory>
#include <variant>
#include <vector>

namespace dsl::ir {

struct NoteVal {
    int midi_note{};
    double duration_beats{1.0};
    int velocity{100};
};

struct RestVal {
    double duration_beats{1.0};
};

struct Value;

struct SeqVal {
    std::vector<std::shared_ptr<Value>> items;
};

struct ChordVal {
    std::vector<NoteVal> notes;
    double duration_beats{1.0};
};

using ValueKind = std::variant<int, double, bool, NoteVal, RestVal, SeqVal, ChordVal>;

struct Value {
    ValueKind kind;
};

}  // namespace dsl::ir
