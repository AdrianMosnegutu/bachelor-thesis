#pragma once

#include <cstdio>
#include <string>

#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/frontend/parse_result.hpp"

namespace dsl::frontend {

[[nodiscard]] ParseResult parse_stream(FILE* input, const std::string& source_name, DiagnosticsEngine& diagnostics);
[[nodiscard]] ParseResult parse_source(const std::string& source,
                                       const std::string& source_name,
                                       DiagnosticsEngine& diagnostics);

}  // namespace dsl::frontend
