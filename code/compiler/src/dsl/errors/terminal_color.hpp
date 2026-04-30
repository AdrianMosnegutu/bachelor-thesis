#pragma once

namespace dsl::errors::detail {

struct TerminalColor {
    static constexpr auto Reset = "\033[0m";
    static constexpr auto Bold = "\033[1m";
    static constexpr auto Red = "\033[31m";
    static constexpr auto Yellow = "\033[33m";
    static constexpr auto Cyan = "\033[36m";
    static constexpr auto BoldRed = "\033[1;31m";
};

}  // namespace dsl::errors::detail
