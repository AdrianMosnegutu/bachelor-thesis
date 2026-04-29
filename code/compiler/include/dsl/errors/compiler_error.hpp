#pragma once

#include <sstream>
#include <stdexcept>
#include <string>

namespace dsl::errors {

class CompilerError : public std::runtime_error {
   public:
    template <typename TLocation>
    CompilerError(const TLocation& loc, const std::string& msg)
        : std::runtime_error(loc_to_string(loc) + ": " + msg), loc_str_(loc_to_string(loc)), msg_(msg) {}

    [[nodiscard]] virtual std::string format() const = 0;

    ~CompilerError() override = default;

   protected:
    std::string loc_str_;
    std::string msg_;

   private:
    template <typename TLocation>
    static std::string loc_to_string(const TLocation& loc) {
        std::ostringstream oss;
        oss << loc;
        return oss.str();
    }
};

}  // namespace dsl::errors
