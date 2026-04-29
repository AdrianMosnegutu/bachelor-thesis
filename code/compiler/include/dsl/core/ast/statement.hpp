#pragma once

#include <memory>
#include <vector>

#include "dsl/core/ast/expression.hpp"
#include "statements/assign_statement.h"
#include "statements/for_statement.h"
#include "statements/if_statement.h"
#include "statements/let_statement.h"
#include "statements/loop_statement.h"
#include "statements/play_statement.h"

namespace dsl::ast {

using StatementKind =
    std::variant<AssignStatement, ForStatement, IfStatement, LetStatement, LoopStatement, PlayStatement>;

struct Statement {
    StatementKind kind;
    Location location;
};

using StatementPtr = std::unique_ptr<Statement>;
using Block = std::vector<StatementPtr>;

}  // namespace dsl::ast
