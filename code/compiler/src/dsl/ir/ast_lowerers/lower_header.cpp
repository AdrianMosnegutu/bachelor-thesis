#include "dsl/ast/program.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir {

void lower_header(const ast::Header& header, ir::Program& out) {
    if (header.tempo) {
        out.tempo_bpm = header.tempo->beats_per_minute;
    }

    if (header.signature) {
        out.time_sig_numerator = header.signature->beats;
        out.time_sig_denominator = header.signature->unit;
    }
}

}  // namespace dsl::ir
