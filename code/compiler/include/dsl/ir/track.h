#pragma once

#include <optional>
#include <string>

#include "dsl/core/music/instrument.hpp"
#include "dsl/ir/note_event.h"

namespace dsl::ir {

struct Track {
    std::optional<std::string> name;
    music::Instrument instrument{music::Instrument::Piano};
    std::vector<NoteEvent> events;  // sorted by start_beat after lowering
};

}  // namespace dsl::ir