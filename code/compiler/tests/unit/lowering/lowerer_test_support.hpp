#pragma once

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>

#include "dsl/common/ast/program.hpp"
#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/common/ir/program.hpp"
#include "dsl/diagnostics/diagnostic.hpp"
#include "dsl/frontend/parse.hpp"
#include "dsl/lowerer/lower.hpp"
#include "dsl/semantic/analyze.hpp"

namespace dsl::testing::lowerer {

inline std::unique_ptr<ast::Program> parse(const std::string& src, DiagnosticsEngine& diagnostics) {
    return frontend::parse_source(src, "<source>", diagnostics).take_program();
}

inline ir::Program lower(const std::string& src) {
    DiagnosticsEngine diagnostics;
    const auto program = parse(src, diagnostics);
    EXPECT_NE(program, nullptr) << "parse failed for: " << src;

    const auto analysis = semantic::analyze(*program, diagnostics);
    for (const auto& diagnostic : diagnostics.diagnostics()) {
        if (diagnostic.is_error()) {
            throw std::runtime_error(format_diagnostic(diagnostic));
        }
    }

    const auto lowered = dsl::lowerer::lower(analysis, diagnostics);
    for (const auto& diagnostic : diagnostics.diagnostics()) {
        if (diagnostic.is_error()) {
            throw std::runtime_error(format_diagnostic(diagnostic));
        }
    }

    return *lowered.program();
}

}  // namespace dsl::testing::lowerer
