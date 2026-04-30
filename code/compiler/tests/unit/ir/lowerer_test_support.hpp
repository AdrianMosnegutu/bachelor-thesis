#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "dsl/ast/program.hpp"
#include "dsl/errors/semantic_error.hpp"
#include "dsl/frontend/parse.hpp"
#include "dsl/ir/lowerer.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/semantic/analyzer.hpp"
#include "dsl/source/location.hpp"

namespace dsl::testing::ir {

inline std::unique_ptr<ast::Program> parse(const std::string& src) { return frontend::parse_source(src).program; }

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
