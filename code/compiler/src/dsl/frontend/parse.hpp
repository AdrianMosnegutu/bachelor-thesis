#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "dsl/ast/program.hpp"

namespace dsl::frontend {

struct ParseResult {
    std::unique_ptr<ast::Program> program;
    std::vector<std::string> errors;

    ParseResult() = default;
    ParseResult(std::unique_ptr<ast::Program> program, std::vector<std::string> errors);

    ParseResult(ParseResult&&) noexcept = default;
    ParseResult(const ParseResult&) = delete;

    ParseResult& operator=(ParseResult&&) noexcept = default;
    ParseResult& operator=(const ParseResult&) = delete;

    ~ParseResult() = default;

    [[nodiscard]] bool ok() const;
};

[[nodiscard]] ParseResult parse_stream(FILE* input, const std::string& source_name);
[[nodiscard]] ParseResult parse_source(const std::string& source, const std::string& source_name = "<source>");

}  // namespace dsl::frontend
