#pragma once

#include <string>

#include "dsl/common/ir/program.hpp"

namespace dsl::midi {

void write_midi(const ir::Program& program, const std::string& output_path);

}  // namespace dsl::midi
