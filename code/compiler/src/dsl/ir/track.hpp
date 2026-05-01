#pragma once

#include <optional>
#include <string>

#include "dsl/ir/note_event.hpp"
#include "dsl/music/instrument.hpp"

namespace dsl::ir {

struct Track {
    std::optional<std::string> name;
    music::Instrument instrument{music::Instrument::Piano};
    NoteEvents events;  // sorted by start_beat after lowering
};

}  // namespace dsl::ir
