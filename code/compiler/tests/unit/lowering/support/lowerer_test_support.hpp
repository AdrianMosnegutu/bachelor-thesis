#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "dsl/common/ast/program.hpp"
#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/common/ir/program.hpp"
#include "dsl/diagnostics/diagnostic.hpp"
#include "dsl/lowering/lower.hpp"
#include "dsl/parsing/parse.hpp"
#include "dsl/semantic/analyze.hpp"

namespace dsl::testing::lowerer {

// -- Result types --------------------------------------------------------------

struct LoweringResult {
    std::unique_ptr<ast::Program> program;
    semantic::AnalysisResult analysis;
    DiagnosticsEngine diagnostics;
    lowering::LowerResult result;
};

// -- Core helpers --------------------------------------------------------------

inline std::unique_ptr<ast::Program> parse(const std::string& src, DiagnosticsEngine& diagnostics) {
    return parsing::parse_source(src, "<test>", diagnostics).take_program();
}

// Lower src through the full pipeline; throws if any error occurs.
// Use this for happy-path output validation tests.
inline ir::Program lower_ok(const std::string& src) {
    DiagnosticsEngine diagnostics;
    auto program = parse(src, diagnostics);
    EXPECT_NE(program, nullptr) << "parse failed for:\n" << src;

    auto analysis = semantic::analyze(*program, diagnostics);
    for (const auto& d : diagnostics.diagnostics()) {
        if (d.is_error()) {
            ADD_FAILURE() << "unexpected error before lowering: " << d.message;
        }
    }

    auto lowered = lowering::lower(analysis, diagnostics);
    for (const auto& d : diagnostics.diagnostics()) {
        if (d.is_error()) {
            ADD_FAILURE() << "unexpected lowering error: " << d.message;
        }
    }

    EXPECT_TRUE(lowered.ok()) << "lowering returned no program for:\n" << src;
    return *lowered.program();
}

// Lower src and expose diagnostics without asserting on them.
// Use this for error-path tests.
inline LoweringResult lower_with_diagnostics(const std::string& src) {
    DiagnosticsEngine diagnostics;
    auto program = parse(src, diagnostics);
    EXPECT_NE(program, nullptr) << "parse failed for:\n" << src;
    auto analysis = semantic::analyze(*program, diagnostics);
    auto result = lowering::lower(analysis, diagnostics);
    return {std::move(program), std::move(analysis), std::move(diagnostics), std::move(result)};
}

// -- Event query helpers -------------------------------------------------------

inline std::vector<int> notes_at(const ir::Track& track, double beat) {
    std::vector<int> result;
    for (const auto& ev : track.events) {
        if (std::abs(ev.start_beat - beat) < 1e-9) result.push_back(ev.midi_note);
    }
    std::ranges::stable_sort(result);
    return result;
}

inline bool has_lowering_error(const Diagnostics& diags, const std::string& text) {
    return std::ranges::any_of(diags, [&](const Diagnostic& d) {
        return d.stage == DiagnosticStage::Lowering && d.is_error() && d.message.find(text) != std::string::npos;
    });
}

}  // namespace dsl::testing::lowerer
