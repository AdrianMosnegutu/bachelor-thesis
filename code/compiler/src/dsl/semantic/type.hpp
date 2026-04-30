#pragma once

#include <string_view>

namespace dsl::semantic {

enum class TypeKind {
    Unknown,
    Void,
    Int,
    Double,
    Bool,
    Note,
    Rest,
    Chord,
    Sequence,
    Drum,
};

struct Type {
    TypeKind kind = TypeKind::Unknown;

    [[nodiscard]] friend bool operator==(const Type&, const Type&) = default;
    [[nodiscard]] bool is_unknown() const { return kind == TypeKind::Unknown; }
};

[[nodiscard]] constexpr std::string_view type_name(const Type type) {
    switch (type.kind) {
        case TypeKind::Unknown:
            return "unknown";
        case TypeKind::Void:
            return "void";
        case TypeKind::Int:
            return "int";
        case TypeKind::Double:
            return "double";
        case TypeKind::Bool:
            return "bool";
        case TypeKind::Note:
            return "note";
        case TypeKind::Rest:
            return "rest";
        case TypeKind::Chord:
            return "chord";
        case TypeKind::Sequence:
            return "seq";
        case TypeKind::Drum:
            return "drum";
    }

    return "unknown";
}

}  // namespace dsl::semantic
