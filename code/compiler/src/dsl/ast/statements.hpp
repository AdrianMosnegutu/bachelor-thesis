#pragma once

#include <memory>
#include <vector>

#include "dsl/ast/expressions.hpp"
#include "dsl/music/drum_note.hpp"

namespace dsl::ast {

struct Statement;
using StatementPtr = std::unique_ptr<Statement>;
using Block = std::vector<StatementPtr>;

// -- Declarative statements -------------------------------------------------------------------------------------------

struct AssignStatement {
    std::string name;
    ExpressionPtr value;
};

struct LetStatement {
    std::string name;
    ExpressionPtr value;
};

// -- Control flow statements ------------------------------------------------------------------------------------------

struct LoopStatement {
    ExpressionPtr count;
    Block body;
};

struct ForStatement {
    StatementPtr init;
    ExpressionPtr condition;  // null for `for (;;)`
    StatementPtr step;
    Block body;
};

struct IfStatement {
    ExpressionPtr condition;
    Block then_branch;
    std::optional<Block> else_branch;
};

// -- Musical statements -----------------------------------------------------------------------------------------------

using PlaySource = std::variant<ExpressionPtr, music::DrumNote>;

struct PlayTarget {
    PlaySource source;
    ExpressionPtr duration;     // null when absent
    ExpressionPtr from_offset;  // null when absent
    source::Location location;
};

struct PlayStatement {
    PlayTarget target;
};

// -- Base statement ---------------------------------------------------------------------------------------------------

using StatementKind =
    std::variant<AssignStatement, ForStatement, IfStatement, LetStatement, LoopStatement, PlayStatement>;

struct Statement {
    StatementKind kind;
    source::Location location;
};

}  // namespace dsl::ast
