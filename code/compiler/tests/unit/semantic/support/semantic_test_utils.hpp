#pragma once

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <string>

#include "dsl/common/ast/program.hpp"
#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/diagnostics/diagnostic.hpp"
#include "dsl/parsing/parse.hpp"
#include "dsl/semantic/analysis_result.hpp"
#include "dsl/semantic/analyze.hpp"

namespace dsl::testing::semantic {

// -- Result types --------------------------------------------------------------

struct OkAnalysis {
    std::unique_ptr<ast::Program> program;
    ::dsl::semantic::AnalysisResult result;
};

struct Analysis {
    std::unique_ptr<ast::Program> program;
    ::dsl::semantic::AnalysisResult result;
    Diagnostics diagnostics;
};

// -- Core helpers --------------------------------------------------------------

inline OkAnalysis analyze_ok(const std::string& src) {
    DiagnosticsEngine engine;
    auto program = parsing::parse_source(src, "<test>", engine).take_program();
    EXPECT_NE(program, nullptr) << "parse failed:\n" << src;
    auto result = ::dsl::semantic::analyze(*program, engine);
    EXPECT_FALSE(engine.has_errors()) << "expected no errors for:\n" << src;
    return {std::move(program), std::move(result)};
}

inline Analysis analyze(const std::string& src) {
    DiagnosticsEngine engine;
    auto program = parsing::parse_source(src, "<test>", engine).take_program();
    if (!program) {
        ADD_FAILURE() << "parse returned null program for:\n" << src;
        DiagnosticsEngine dummy;
        auto dummy_prog = std::make_unique<ast::Program>();
        auto dummy_result = ::dsl::semantic::analyze(*dummy_prog, dummy);
        return {std::move(dummy_prog), std::move(dummy_result), engine.take_diagnostics()};
    }
    auto result = ::dsl::semantic::analyze(*program, engine);
    return {std::move(program), std::move(result), engine.take_diagnostics()};
}

// analyze_expecting_parse_error: use for inputs that should fail at parse time.
// Does NOT call ADD_FAILURE when the program is null — that is the expected outcome.
inline Diagnostics analyze_expecting_parse_error(const std::string& src) {
    DiagnosticsEngine engine;
    (void)parsing::parse_source(src, "<test>", engine);
    return engine.take_diagnostics();
}

// -- Diagnostic query helpers --------------------------------------------------

inline bool has_parse_error(const Diagnostics& diags) {
    return std::ranges::any_of(diags,
                               [](const Diagnostic& d) { return d.stage == DiagnosticStage::Parsing && d.is_error(); });
}

inline bool has_semantic_error(const Diagnostics& diags) {
    return std::ranges::any_of(diags, [](const Diagnostic& d) {
        return d.stage == DiagnosticStage::Semantic && d.is_error();
    });
}

inline bool has_semantic_warning(const Diagnostics& diags) {
    return std::ranges::any_of(diags, [](const Diagnostic& d) {
        return d.stage == DiagnosticStage::Semantic && d.severity == DiagnosticSeverity::Warning;
    });
}

inline bool has_error(const Diagnostics& diags, const std::string& text) {
    return std::ranges::any_of(diags, [&](const Diagnostic& d) {
        return d.is_error() && d.message.find(text) != std::string::npos;
    });
}

inline bool has_warning(const Diagnostics& diags, const std::string& text) {
    return std::ranges::any_of(diags, [&](const Diagnostic& d) {
        return d.severity == DiagnosticSeverity::Warning && d.message.find(text) != std::string::npos;
    });
}

inline const Diagnostic* find_error(const Diagnostics& diags, const std::string& text) {
    for (const auto& d : diags) {
        if (d.is_error() && d.message.find(text) != std::string::npos) return &d;
    }
    return nullptr;
}

inline const Diagnostic* find_warning(const Diagnostics& diags, const std::string& text) {
    for (const auto& d : diags) {
        if (d.severity == DiagnosticSeverity::Warning && d.message.find(text) != std::string::npos) return &d;
    }
    return nullptr;
}

inline std::size_t count_semantic_errors(const Diagnostics& diags) {
    return static_cast<std::size_t>(std::ranges::count_if(diags, [](const Diagnostic& d) {
        return d.stage == DiagnosticStage::Semantic && d.is_error();
    }));
}

}  // namespace dsl::testing::semantic
