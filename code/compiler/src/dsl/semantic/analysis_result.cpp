#include "dsl/semantic/analysis_result.hpp"

#include <memory>

#include "dsl/common/ast/program.hpp"
#include "dsl/semantic/annotations.hpp"
#include "dsl/semantic/symbol_table.hpp"

namespace dsl::semantic {

AnalysisResult::AnalysisResult(const ast::Program& program)
    : program_(&program), symbols_(std::make_unique<SymbolTable>()), annotations_(std::make_unique<Annotations>()) {}

const ast::Program& AnalysisResult::program() const { return *program_; }

const SymbolTable& AnalysisResult::symbols() const { return *symbols_; }
SymbolTable& AnalysisResult::symbols() { return *symbols_; }

const Annotations& AnalysisResult::annotations() const { return *annotations_; }
Annotations& AnalysisResult::annotations() { return *annotations_; }

}  // namespace dsl::semantic
