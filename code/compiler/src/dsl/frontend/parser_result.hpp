#pragma once

#include <memory>

#include "dsl/ast/program.hpp"

namespace dsl::frontend {

class ParseResult {
   public:
    explicit ParseResult(std::unique_ptr<ast::Program> program);

    ParseResult(ParseResult&&) noexcept = default;
    ParseResult(const ParseResult&) = delete;

    ParseResult& operator=(ParseResult&&) noexcept = default;
    ParseResult& operator=(const ParseResult&) = delete;

    ~ParseResult() = default;

    [[nodiscard]] bool ok() const;

    [[nodiscard]] const ast::Program* program() const;
    [[nodiscard]] std::unique_ptr<ast::Program> take_program();

   private:
    std::unique_ptr<ast::Program> program_;
};

}  // namespace dsl::frontend
