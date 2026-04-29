#pragma once

#include <string>

#include "dsl/ir/program.hpp"

namespace dsl::backend {

class MidiWriter {
   public:
    static void write(const ir::ProgramIR& program, const std::string& output_path);
};

}  // namespace dsl::backend
