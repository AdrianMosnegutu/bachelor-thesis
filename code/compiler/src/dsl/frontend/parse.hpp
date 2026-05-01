#pragma once

#include <cstdio>
#include <string>

#include "dsl/diagnostics/diagnostics_engine.hpp"
#include "dsl/frontend/parser_result.hpp"

namespace dsl::frontend {

[[nodiscard]] ParseResult parse_stream(FILE* input, const std::string& source_name, DiagnosticsEngine& diagnostics);
[[nodiscard]] ParseResult parse_source(const std::string& source,
                                       const std::string& source_name,
                                       DiagnosticsEngine& diagnostics);

}  // namespace dsl::frontend
