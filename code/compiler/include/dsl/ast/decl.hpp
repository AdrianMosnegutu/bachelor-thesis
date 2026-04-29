#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "dsl/ast/stmt.hpp"
#include "dsl/music/instrument.hpp"
#include "dsl/music/key_mode.hpp"

namespace dsl::ast {

struct TempoDeclaration {
    int bpm{};
    Location loc;
};

struct SignatureDeclaration {
    int beats{};
    int unit{};
    Location loc;
};

struct KeyDeclaration {
    music::PitchClass pitch;
    music::KeyMode mode;
    Location loc;
};

struct PatternDefinition {
    std::string name;
    std::vector<std::string> params;
    Block body;
    Location loc;
};

struct Header {
    std::optional<TempoDeclaration> tempo;
    std::optional<SignatureDeclaration> signature;
    std::optional<KeyDeclaration> key;
};

// Global scope: only `let` statements and pattern definitions.
using GlobalItem = std::variant<StatementPtr, PatternDefinition>;

// Voice scope: statements + local pattern definitions (no nested voice).
using VoiceItem = std::variant<StatementPtr, PatternDefinition>;

struct VoiceDeclaration {
    std::optional<ExpressionPtr> from_expr;
    std::vector<VoiceItem> body;
    Location loc;
};

// Track scope: statements, local pattern definitions, and voice declarations.
using TrackItem = std::variant<StatementPtr, PatternDefinition, VoiceDeclaration>;

struct TrackDeclaration {
    std::optional<std::string> name;
    std::optional<music::Instrument> instrument;
    std::vector<TrackItem> body;
    Location loc;
};

}  // namespace dsl::ast
