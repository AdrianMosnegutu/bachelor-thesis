#pragma once

#include <memory>
#include <vector>

namespace dsl::ir::detail {

struct Value;

struct SequenceValue {
    std::vector<std::shared_ptr<Value>> items;
};

}  // namespace dsl::ir::detail
