#pragma once

#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace dsl::ast {

struct Program;

}

namespace dsl::frontend {

struct ParseResult {
    std::unique_ptr<ast::Program> program;
    std::vector<std::string> errors;

    ParseResult();
    ParseResult(std::unique_ptr<ast::Program> program, std::vector<std::string> errors);

    ParseResult(ParseResult&&) noexcept;
    ParseResult(const ParseResult&) = delete;

    ParseResult& operator=(ParseResult&&) noexcept;
    ParseResult& operator=(const ParseResult&) = delete;

    ~ParseResult();

    [[nodiscard]] bool ok() const { return program != nullptr && errors.empty(); }
};

[[nodiscard]] ParseResult parse_stream(FILE* input, const std::string& source_name);
[[nodiscard]] ParseResult parse_source(const std::string& source, const std::string& source_name = "<source>");

}  // namespace dsl::frontend
