#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "dsl/core/ast/statement.hpp"
#include "dsl/core/location.hpp"
#include "dsl/core/music/instrument.hpp"
#include "pattern_definition.h"
#include "voice_definition.h"

namespace dsl::ast {

using TrackItem = std::variant<StatementPtr, PatternDefinition, VoiceDefinition>;

struct TrackDefinition {
    std::optional<std::string> name;
    std::optional<music::Instrument> instrument;
    std::vector<TrackItem> body;
    Location location;
};

}  // namespace dsl::ast