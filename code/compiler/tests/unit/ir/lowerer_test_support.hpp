#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "dsl/core/ast/program.hpp"
#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/core/location.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/semantic/analyzer.hpp"
#include "parser.hpp"

struct yy_buffer_state;
using YY_BUFFER_STATE = yy_buffer_state*;

YY_BUFFER_STATE yy_scan_string(const char* str);
void yy_delete_buffer(YY_BUFFER_STATE buf);
void scanner_reset();

namespace dsl::testing::ir {

struct ParseGuard {
    YY_BUFFER_STATE buf;

    explicit ParseGuard(const std::string& src) {
        scanner_reset();
        buf = yy_scan_string(src.c_str());
    }

    ~ParseGuard() {
        yy_delete_buffer(buf);
        scanner_reset();
    }

    ParseGuard(const ParseGuard&) = delete;
    ParseGuard& operator=(const ParseGuard&) = delete;
};

inline std::unique_ptr<ast::Program> parse(const std::string& src) {
    ParseGuard guard(src);
    auto program = std::make_unique<ast::Program>();
    Location loc;
    frontend::Parser parser{loc, *program};
    return parser.parse() == 0 ? std::move(program) : nullptr;
}

inline dsl::ir::Program lower(const std::string& src) {
    const auto program = parse(src);
    EXPECT_NE(program, nullptr) << "parse failed for: " << src;

    const auto analysis = dsl::semantic::analyze(*program);
    for (const auto& diagnostic : analysis.diagnostics()) {
        if (diagnostic.is_error()) {
            throw dsl::errors::SemanticError(diagnostic.location, diagnostic.message);
        }
    }

    return dsl::ir::lower(analysis);
}

}  // namespace dsl::testing::ir
