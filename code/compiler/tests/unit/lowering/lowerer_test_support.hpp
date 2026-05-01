#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "dsl/ast/program.hpp"
#include "dsl/errors/semantic_error.hpp"
#include "dsl/frontend/parse.hpp"
#include "dsl/ir/program.hpp"
#include "dsl/lowerer/lowerer.hpp"
#include "dsl/semantic/analyzer.hpp"

namespace dsl::testing::lowerer {

inline std::unique_ptr<ast::Program> parse(const std::string& src) { return frontend::parse_source(src).program; }

inline ir::Program lower(const std::string& src) {
    const auto program = parse(src);
    EXPECT_NE(program, nullptr) << "parse failed for: " << src;

    const auto analysis = semantic::analyze(*program);
    for (const auto& diagnostic : analysis.diagnostics()) {
        if (diagnostic.is_error()) {
            throw errors::SemanticError(diagnostic.location, diagnostic.message);
        }
    }

    return ::dsl::lowerer::lower(analysis);
}

}  // namespace dsl::testing::lowerer
