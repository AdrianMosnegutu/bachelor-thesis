#pragma once

#include <optional>
#include <string>

#include "dsl/common/ir/note_event.hpp"
#include "dsl/common/music/instrument.hpp"

namespace dsl::ir {

struct Track {
    std::optional<std::string> name;
    music::Instrument instrument{music::Instrument::Piano};
    NoteEvents events;  // sorted by start_beat after lowering
};

}  // namespace dsl::ir
