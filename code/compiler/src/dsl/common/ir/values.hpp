#pragma once

#include <memory>
#include <variant>
#include <vector>

namespace dsl::ir {

struct Value;

struct NoteValue {
    int midi_note{};
    double duration_beats{1.0};
    int velocity{100};
};

struct RestValue {
    double duration_beats{1.0};
};

struct ChordValue {
    std::vector<NoteValue> notes;
    // 0.0 = sentinel meaning "derive from max note duration"; any other value is explicit.
    double duration_beats{0.0};
};

struct SequenceValue {
    std::vector<std::shared_ptr<Value>> items;
};

using ValueKind = std::variant<int, double, bool, NoteValue, RestValue, SequenceValue, ChordValue>;

struct Value {
    ValueKind kind;
};

}  // namespace dsl::ir
