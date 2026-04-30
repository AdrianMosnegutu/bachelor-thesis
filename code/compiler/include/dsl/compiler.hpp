#pragma once

#include <cstdio>
#include <string>
#include <vector>

namespace dsl {

enum class CompileStage {
    Frontend,
    Semantic,
    Lowering,
    Backend,
};

struct CompileMessage {
    CompileStage stage;
    std::string message;
};

struct CompileResult {
    std::vector<CompileMessage> errors;

    [[nodiscard]] bool ok() const { return errors.empty(); }
};

[[nodiscard]] const char* to_string(CompileStage stage);

[[nodiscard]] CompileResult compile(FILE* input, const std::string& source_name, const std::string& output_path);

}  // namespace dsl
