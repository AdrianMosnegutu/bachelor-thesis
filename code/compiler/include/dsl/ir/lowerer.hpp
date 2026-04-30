#pragma once

#include "dsl/core/ast/program.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir {

Program lower(const ast::Program& program);

}  // namespace dsl::ir
