#pragma once

#include <optional>

#include "dsl/common/ir/program.hpp"

namespace dsl::lowerer {

class LowerResult {
   public:
    explicit LowerResult(std::optional<ir::Program> program);

    [[nodiscard]] bool ok() const;
    [[nodiscard]] const std::optional<ir::Program>& program() const;

   private:
    std::optional<ir::Program> program_;
};

}  // namespace dsl::lowerer
