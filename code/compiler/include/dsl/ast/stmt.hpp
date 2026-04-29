#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "dsl/ast/expr.hpp"
#include "dsl/ast/play.hpp"

namespace dsl::ast {

struct Statement {
    Location loc;

    explicit Statement(const Location& loc) : loc(loc) {}
    virtual ~Statement() = default;
};

using StatementPtr = std::unique_ptr<Statement>;
using Block = std::vector<StatementPtr>;

struct LetStatement : Statement {
    std::string name;
    ExpressionPtr value;

    LetStatement(std::string name, ExpressionPtr value, const Location& loc)
        : Statement(loc), name(std::move(name)), value(std::move(value)) {}
};

struct AssignStatement : Statement {
    std::string name;
    ExpressionPtr value;

    AssignStatement(std::string name, ExpressionPtr value, const Location& loc)
        : Statement(loc), name(std::move(name)), value(std::move(value)) {}
};

struct PlayStatement : Statement {
    PlayTarget target;

    PlayStatement(PlayTarget target, const Location& loc) : Statement(loc), target(std::move(target)) {}
};

// `init` is LetStmt or AssignStmt (null when omitted); `step` is AssignStmt (null when omitted).
struct ForStatement : Statement {
    StatementPtr init;
    ExpressionPtr cond;  // null for `for (;;)`
    StatementPtr step;
    Block body;

    ForStatement(StatementPtr init, ExpressionPtr cond, StatementPtr step, Block body, const Location& loc)
        : Statement(loc), init(std::move(init)), cond(std::move(cond)), step(std::move(step)), body(std::move(body)) {}
};

struct IfStatement : Statement {
    ExpressionPtr cond;
    Block then_branch;
    std::optional<Block> else_branch;

    IfStatement(ExpressionPtr cond, Block then_branch, const Location& loc)
        : Statement(loc), cond(std::move(cond)), then_branch(std::move(then_branch)) {}

    IfStatement(ExpressionPtr cond, Block then_branch, Block else_branch, const Location& loc)
        : Statement(loc),
          cond(std::move(cond)),
          then_branch(std::move(then_branch)),
          else_branch(std::move(else_branch)) {}
};

struct LoopStatement : Statement {
    ExpressionPtr count;
    Block body;

    LoopStatement(ExpressionPtr count, Block body, const Location& loc)
        : Statement(loc), count(std::move(count)), body(std::move(body)) {}
};

}  // namespace dsl::ast
