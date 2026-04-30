#pragma once

#include <memory>
#include <vector>

#include "statements/assign_statement.hpp"
#include "statements/for_statement.hpp"
#include "statements/if_statement.hpp"
#include "statements/let_statement.hpp"
#include "statements/loop_statement.hpp"
#include "statements/play_statement.hpp"

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
