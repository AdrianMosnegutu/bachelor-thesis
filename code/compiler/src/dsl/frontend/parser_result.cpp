#include "parser_result.hpp"

#include <utility>

#include "dsl/common/ast/program.hpp"

namespace dsl::frontend {

ParseResult::ParseResult(std::unique_ptr<ast::Program> program) : program_(std::move(program)) {}

bool ParseResult::ok() const { return program_ != nullptr; }

const ast::Program* ParseResult::program() const { return program_.get(); }

std::unique_ptr<ast::Program> ParseResult::take_program() { return std::move(program_); }

}  // namespace dsl::frontend
