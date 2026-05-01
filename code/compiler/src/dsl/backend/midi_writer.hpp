#pragma once

#include <string>

#include "dsl/common/ir/program.hpp"

namespace dsl::backend {

class MidiWriter {
   public:
    static void write(const ir::Program& program, const std::string& output_path);
};

}  // namespace dsl::backend
