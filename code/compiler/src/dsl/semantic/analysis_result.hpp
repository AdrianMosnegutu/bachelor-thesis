#pragma once

#include <memory>
#include <vector>

#include "dsl/semantic/diagnostic.hpp"

namespace dsl::ast {
struct Program;
}

namespace dsl::semantic {

class Annotations;
class SymbolTable;

class AnalysisResult {
   public:
    explicit AnalysisResult(const ast::Program& program);

    AnalysisResult(AnalysisResult&&) noexcept;
    AnalysisResult& operator=(AnalysisResult&&) noexcept;
    ~AnalysisResult();

    [[nodiscard]] const ast::Program& program() const;

    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }
    [[nodiscard]] std::vector<Diagnostic>& diagnostics() { return diagnostics_; }

    [[nodiscard]] const SymbolTable& symbols() const;
    [[nodiscard]] SymbolTable& symbols();

    [[nodiscard]] const Annotations& annotations() const;
    [[nodiscard]] Annotations& annotations();

    [[nodiscard]] bool has_errors() const;
    [[nodiscard]] bool ok() const { return !has_errors(); }

   private:
    const ast::Program* program_;
    std::vector<Diagnostic> diagnostics_;
    std::unique_ptr<SymbolTable> symbols_;
    std::unique_ptr<Annotations> annotations_;
};

}  // namespace dsl::semantic
