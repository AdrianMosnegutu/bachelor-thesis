#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "declarations/signature_declaration.hpp"
#include "declarations/tempo_declaration.hpp"
#include "definitions/track_definition.hpp"

namespace dsl::ast {

struct Header {
    std::optional<TempoDeclaration> tempo;
    std::optional<SignatureDeclaration> signature;
};

using GlobalItem = std::variant<StatementPtr, PatternDefinition>;

struct Program {
    Header header;
    std::vector<GlobalItem> globals;
    std::vector<TrackDefinition> tracks;
};

}  // namespace dsl::ast
