#include "dsl/semantic/analysis_result.hpp"

#include <algorithm>
#include <memory>

#include "dsl/semantic/annotations.hpp"
#include "dsl/semantic/symbol_table.hpp"

namespace dsl::semantic {

AnalysisResult::AnalysisResult(const ast::Program& program)
    : program_(&program), symbols_(std::make_unique<SymbolTable>()), annotations_(std::make_unique<Annotations>()) {}

AnalysisResult::AnalysisResult(AnalysisResult&&) noexcept = default;
AnalysisResult& AnalysisResult::operator=(AnalysisResult&&) noexcept = default;
AnalysisResult::~AnalysisResult() = default;

const ast::Program& AnalysisResult::program() const { return *program_; }

const SymbolTable& AnalysisResult::symbols() const { return *symbols_; }
SymbolTable& AnalysisResult::symbols() { return *symbols_; }

const Annotations& AnalysisResult::annotations() const { return *annotations_; }
Annotations& AnalysisResult::annotations() { return *annotations_; }

bool AnalysisResult::has_errors() const {
    return std::ranges::any_of(diagnostics_, [](const Diagnostic& d) { return d.is_error(); });
}

}  // namespace dsl::semantic
