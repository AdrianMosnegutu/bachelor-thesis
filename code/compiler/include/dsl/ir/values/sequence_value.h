#pragma once

#include <memory>
#include <vector>

namespace dsl::ir {

struct Value;

struct SequenceValue {
    std::vector<std::shared_ptr<Value>> items;
};

}  // namespace dsl::ir
