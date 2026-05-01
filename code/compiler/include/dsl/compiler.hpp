#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace dsl {

enum class CompileStage : uint8_t {
    Frontend,
    Semantic,
    Lowering,
    Backend,
};

struct Diagnostic {
    CompileStage stage;
    std::string message;
};

using Diagnostics = std::vector<Diagnostic>;

class CompileResult {
   public:
    [[nodiscard]] bool ok() const;

    void add_diagnostic(const Diagnostic& diagnostic);
    void add_diagnostic(CompileStage stage, const std::string& message);

    [[nodiscard]] const Diagnostics& get_diagnostics() const;

   private:
    Diagnostics diagnostics_;
};

[[nodiscard]] const char* to_string(CompileStage stage);

[[nodiscard]] CompileResult compile(FILE* input, const std::string& source_name, const std::string& output_path);

}  // namespace dsl
