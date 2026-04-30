#pragma once

#include <stdexcept>
#include <string>

#include "dsl/source/location.hpp"

namespace dsl::errors {

class CompilerError : public std::runtime_error {
   public:
    CompilerError(const Location& loc, const std::string& msg);
    ~CompilerError() override = default;

    [[nodiscard]] virtual std::string format() const = 0;

   protected:
    std::string loc_str_;
    std::string msg_;

   private:
    static std::string loc_to_string(const Location& loc);
};

}  // namespace dsl::errors
