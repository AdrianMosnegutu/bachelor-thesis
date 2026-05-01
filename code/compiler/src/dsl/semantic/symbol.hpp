#pragma once

#include <cstddef>
#include <limits>
#include <string>

#include "dsl/semantic/type.hpp"
#include "dsl/source/location.hpp"

namespace dsl::semantic {

using SymbolId = std::size_t;

inline constexpr SymbolId INVALID_SYMBOL_ID = std::numeric_limits<SymbolId>::max();

enum class SymbolKind {
    Variable,
    Parameter,
    Pattern,
    Track,
    Voice,
};

struct Symbol {
    SymbolId id = INVALID_SYMBOL_ID;
    std::string name;
    SymbolKind kind = SymbolKind::Variable;
    Type type;
    source::Location location;
    const void* declaration = nullptr;
};

}  // namespace dsl::semantic
