#include <variant>

#include "dsl/common/ir/values.hpp"
#include "dsl/lowering/detail/expression_evaluator.hpp"
#include "dsl/lowering/detail/lowerer_context.hpp"

namespace dsl::lowering::detail {

namespace {

using ir::Value;
using ir::ValueKind;

template <typename T>
inline int as(const ValueKind& kind) {
    return std::get<T>(kind);
}

double cast_to_double(const ValueKind& kind) {
    if (const auto* integer = std::get_if<int>(&kind)) {
        return *integer;
    }

    return std::get<double>(kind);
}

inline bool both_int(const ValueKind& a, const ValueKind& b) {
    return std::holds_alternative<int>(a) && std::holds_alternative<int>(b);
}

inline bool is_bool(const ValueKind& kind) { return std::holds_alternative<bool>(kind); }

}  // namespace

Value evaluate_binary_expression(const ast::BinaryExpression& binary,
                                 const source::Location& loc,
                                 LowererContext& context) {
    using Op = ast::BinaryOperator;

    ValueKind lhs = evaluate_expression(*binary.left, context).kind;
    ValueKind rhs = evaluate_expression(*binary.right, context).kind;

    switch (binary.operation) {
        case Op::Add: {
            return both_int(lhs, rhs) ? Value{as<int>(lhs) + as<int>(rhs)} : Value{as<double>(lhs) + as<double>(rhs)};
        }
        case Op::Subtract: {
            return both_int(lhs, rhs) ? Value{as<int>(lhs) - as<int>(rhs)} : Value{as<double>(lhs) - as<double>(rhs)};
        }
        case Op::Multiply: {
            return both_int(lhs, rhs) ? Value{as<int>(lhs) * as<int>(rhs)} : Value{as<double>(lhs) * as<double>(rhs)};
        }
        case Op::Divide: {
            const double right_raw = cast_to_double(rhs);
            if (right_raw == 0.0) {
                throw LoweringFailure(loc, "division by zero");
            }

            return Value{cast_to_double(lhs) / right_raw};
        }
        case Op::Modulo: {
            const int right_raw = as<int>(rhs);
            if (right_raw == 0) {
                throw LoweringFailure(loc, "modulo by zero");
            }

            return Value{as<int>(lhs) % right_raw};
        }
        case Op::Equals: {
            if (both_int(lhs, rhs)) {
                return Value{as<int>(lhs) == as<int>(rhs)};
            }

            if (std::holds_alternative<bool>(lhs)) {
                return Value{as<bool>(lhs) == as<bool>(rhs)};
            }

            return Value{cast_to_double(lhs) == cast_to_double(rhs)};
        }
        case Op::NotEquals: {
            if (both_int(lhs, rhs)) {
                return Value{std::get<int>(lhs) != std::get<int>(rhs)};
            }

            if (is_bool(lhs)) {
                return Value{std::get<bool>(lhs) != std::get<bool>(rhs)};
            }

            return Value{cast_to_double(lhs) != cast_to_double(rhs)};
        }
        case Op::Less: {
            return Value{cast_to_double(lhs) < cast_to_double(rhs)};
        }
        case Op::Greater: {
            return Value{cast_to_double(lhs) > cast_to_double(rhs)};
        }
        case Op::LessOrEqual: {
            return Value{cast_to_double(lhs) <= cast_to_double(rhs)};
        }
        case Op::GreaterOrEqual: {
            return Value{cast_to_double(lhs) >= cast_to_double(rhs)};
        }
        case Op::And: {
            return Value{as<bool>(lhs) && as<bool>(rhs)};
        }
        case Op::Or: {
            return Value{as<bool>(lhs) || as<bool>(rhs)};
        }
    }

    throw LoweringFailure(loc, "lowering reached invalid binary operator");
}

}  // namespace dsl::lowering::detail
