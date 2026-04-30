#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "dsl/ast/statement.hpp"
#include "dsl/music/instrument.hpp"
#include "dsl/source/location.hpp"
#include "pattern_definition.hpp"
#include "voice_definition.hpp"

namespace dsl::ast {

using TrackItem = std::variant<StatementPtr, PatternDefinition, VoiceDefinition>;

struct TrackDefinition {
    std::optional<std::string> name;
    std::optional<music::Instrument> instrument;
    std::vector<TrackItem> body;
    Location location;
};

}  // namespace dsl::ast
