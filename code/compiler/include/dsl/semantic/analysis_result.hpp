#pragma once

#include <vector>

#include "dsl/core/ast/program.hpp"
#include "dsl/semantic/annotations.hpp"
#include "dsl/semantic/diagnostic.hpp"
#include "dsl/semantic/symbol_table.hpp"

namespace dsl::semantic {

class AnalysisResult {
   public:
    explicit AnalysisResult(const ast::Program& program);

    [[nodiscard]] const ast::Program& program() const { return *program_; }
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }
    [[nodiscard]] std::vector<Diagnostic>& diagnostics() { return diagnostics_; }

    [[nodiscard]] const SymbolTable& symbols() const { return symbols_; }
    [[nodiscard]] SymbolTable& symbols() { return symbols_; }

    [[nodiscard]] const Annotations& annotations() const { return annotations_; }
    [[nodiscard]] Annotations& annotations() { return annotations_; }

    [[nodiscard]] bool has_errors() const;
    [[nodiscard]] bool ok() const { return !has_errors(); }

   private:
    const ast::Program* program_;
    std::vector<Diagnostic> diagnostics_;
    SymbolTable symbols_;
    Annotations annotations_;
};

}  // namespace dsl::semantic
