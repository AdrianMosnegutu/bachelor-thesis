#include "dsl/lowerer/lower_result.hpp"

namespace dsl::lowerer {

LowerResult::LowerResult(std::optional<ir::Program> program) : program_(std::move(program)) {}

bool LowerResult::ok() const { return program_.has_value(); }

const std::optional<ir::Program>& LowerResult::program() const { return program_; }

}  // namespace dsl::lowerer
