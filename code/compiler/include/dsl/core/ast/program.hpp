#pragma once

#include <optional>
#include <variant>
#include <vector>

#include "declarations/key_declaration.h"
#include "declarations/signature_declaration.h"
#include "declarations/tempo_declaration.h"
#include "definitions/track_definition.h"

namespace dsl::ast {

struct Header {
    std::optional<TempoDeclaration> tempo;
    std::optional<SignatureDeclaration> signature;
    std::optional<KeyDeclaration> key;
};

using GlobalItem = std::variant<StatementPtr, PatternDefinition>;

struct Program {
    Header header;
    std::vector<GlobalItem> globals;
    std::vector<TrackDefinition> tracks;
};

}  // namespace dsl::ast
