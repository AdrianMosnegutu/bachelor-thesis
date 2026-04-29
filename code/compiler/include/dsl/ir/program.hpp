#pragma once

#include <vector>

#include "track.h"

namespace dsl::ir {

struct Program {
    int tempo_bpm{120};
    int time_sig_numerator{4};
    int time_sig_denominator{4};
    std::vector<Track> tracks;
};

}  // namespace dsl::ir
