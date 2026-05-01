#pragma once

#include "dsl/ast/program.hpp"
#include "dsl/semantic/annotations.hpp"
#include "dsl/semantic/symbol_table.hpp"

namespace dsl::semantic {

class AnalysisResult {
   public:
    explicit AnalysisResult(const ast::Program& program);

    AnalysisResult(AnalysisResult&&) noexcept = default;
    AnalysisResult(const AnalysisResult&) = delete;

    AnalysisResult& operator=(AnalysisResult&&) noexcept = default;
    AnalysisResult& operator=(const AnalysisResult&) = delete;

    ~AnalysisResult() = default;

    [[nodiscard]] const ast::Program& program() const;

    [[nodiscard]] const SymbolTable& symbols() const;
    [[nodiscard]] SymbolTable& symbols();

    [[nodiscard]] const Annotations& annotations() const;
    [[nodiscard]] Annotations& annotations();

   private:
    const ast::Program* program_;
    std::unique_ptr<SymbolTable> symbols_;
    std::unique_ptr<Annotations> annotations_;
};

}  // namespace dsl::semantic
