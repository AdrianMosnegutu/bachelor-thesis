#include <variant>

#include "dsl/core/errors/semantic_error.hpp"
#include "dsl/ir/lowerer/expression_evaluator.hpp"

namespace dsl::ir::detail {

using errors::SemanticError;

namespace {

double as_double(const ValueKind& kind, const char* side, const Location& loc) {
    if (auto* integer = std::get_if<int>(&kind)) {
        return *integer;
    }

    if (auto* floating_point = std::get_if<double>(&kind)) {
        return *floating_point;
    }

    throw SemanticError(loc, std::string(side) + " operand must be numeric");
}

bool both_int(const ValueKind& a, const ValueKind& b) {
    return std::holds_alternative<int>(a) && std::holds_alternative<int>(b);
}

Value evaluate_add(const ValueKind& left, const ValueKind& right, const Location& loc) {
    if (both_int(left, right)) {
        return Value{std::get<int>(left) + std::get<int>(right)};
    }

    return Value{as_double(left, "left", loc) + as_double(right, "right", loc)};
}

Value evaluate_subtract(const ValueKind& left, const ValueKind& right, const Location& loc) {
    if (both_int(left, right)) {
        return Value{std::get<int>(left) - std::get<int>(right)};
    }

    return Value{as_double(left, "left", loc) - as_double(right, "right", loc)};
}

Value evaluate_multiply(const ValueKind& left, const ValueKind& right, const Location& loc) {
    if (both_int(left, right)) {
        return Value{std::get<int>(left) * std::get<int>(right)};
    }

    return Value{as_double(left, "left", loc) * as_double(right, "right", loc)};
}

Value evaluate_divide(const ValueKind& left, const ValueKind& right, const Location& loc) {
    const double right_raw = as_double(right, "right", loc);
    if (right_raw == 0.0) {
        throw SemanticError(loc, "division by zero");
    }

    return Value{as_double(left, "left", loc) / right_raw};
}

Value evaluate_modulo(const ValueKind& left, const ValueKind& right, const Location& loc) {
    if (!both_int(left, right)) {
        throw SemanticError(loc, "modulo requires integer operands");
    }

    const int right_raw = std::get<int>(right);
    if (right_raw == 0) {
        throw SemanticError(loc, "modulo by zero");
    }

    return Value{std::get<int>(left) % right_raw};
}

Value evaluate_equals(const ValueKind& left, const ValueKind& right, const Location& loc) {
    if (both_int(left, right)) {
        return Value{std::get<int>(left) == std::get<int>(right)};
    }

    if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
        return Value{std::get<bool>(left) == std::get<bool>(right)};
    }

    if ((std::holds_alternative<int>(left) || std::holds_alternative<double>(left)) &&
        (std::holds_alternative<int>(right) || std::holds_alternative<double>(right))) {
        return Value{as_double(left, "left", loc) == as_double(right, "right", loc)};
    }

    throw SemanticError(loc, "'==' requires numeric or boolean operands");
}

Value evaluate_not_equals(const ValueKind& left, const ValueKind& right, const Location& loc) {
    if (both_int(left, right)) {
        return Value{std::get<int>(left) != std::get<int>(right)};
    }

    if (std::holds_alternative<bool>(left) && std::holds_alternative<bool>(right)) {
        return Value{std::get<bool>(left) != std::get<bool>(right)};
    }

    if ((std::holds_alternative<int>(left) || std::holds_alternative<double>(left)) &&
        (std::holds_alternative<int>(right) || std::holds_alternative<double>(right))) {
        return Value{as_double(left, "left", loc) != as_double(right, "right", loc)};
    }

    throw SemanticError(loc, "'==' requires numeric or boolean operands");
}

Value evaluate_less(const ValueKind& left, const ValueKind& right, const Location& loc) {
    return Value{as_double(left, "left", loc) < as_double(right, "right", loc)};
}

Value evaluate_greater(const ValueKind& left, const ValueKind& right, const Location& loc) {
    return Value{as_double(left, "left", loc) > as_double(right, "right", loc)};
}

Value evaluate_less_or_equal(const ValueKind& left, const ValueKind& right, const Location& loc) {
    return Value{as_double(left, "left", loc) <= as_double(right, "right", loc)};
}

Value evaluate_greater_or_equal(const ValueKind& left, const ValueKind& right, const Location& loc) {
    return Value{as_double(left, "left", loc) >= as_double(right, "right", loc)};
}

Value evaluate_and(const ValueKind& left, const ValueKind& right, const Location& loc) {
    auto* left_bool = std::get_if<bool>(&left);
    auto* right_bool = std::get_if<bool>(&right);

    if (!left_bool || !right_bool) {
        throw SemanticError(loc, "'&&' requires boolean operands");
    }

    return Value{*left_bool && *right_bool};
}

Value evaluate_or(const ValueKind& left, const ValueKind& right, const Location& loc) {
    auto* left_bool = std::get_if<bool>(&left);
    auto* right_bool = std::get_if<bool>(&right);

    if (!left_bool || !right_bool) {
        throw SemanticError(loc, "'&&' requires boolean operands");
    }

    return Value{*left_bool || *right_bool};
}

}  // namespace

Value evaluate_binary_expression(const ast::BinaryExpression& binary, const Location& loc, LowererContext& context) {
    using Op = ast::BinaryOperator;

    ValueKind lhs = evaluate_expression(*binary.left, context).kind;
    ValueKind rhs = evaluate_expression(*binary.right, context).kind;

    switch (binary.operation) {
        case Op::Add:
            return evaluate_add(lhs, rhs, loc);
        case Op::Subtract:
            return evaluate_subtract(lhs, rhs, loc);
        case Op::Multiply:
            return evaluate_multiply(lhs, rhs, loc);
        case Op::Divide:
            return evaluate_divide(lhs, rhs, loc);
        case Op::Modulo:
            return evaluate_modulo(lhs, rhs, loc);
        case Op::Equals:
            return evaluate_equals(lhs, rhs, loc);
        case Op::NotEquals:
            return evaluate_not_equals(lhs, rhs, loc);
        case Op::Less:
            return evaluate_less(lhs, rhs, loc);
        case Op::Greater:
            return evaluate_greater(lhs, rhs, loc);
        case Op::LessOrEqual:
            return evaluate_less_or_equal(lhs, rhs, loc);
        case Op::GreaterOrEqual:
            return evaluate_greater_or_equal(lhs, rhs, loc);
        case Op::And:
            return evaluate_and(lhs, rhs, loc);
        case Op::Or:
            return evaluate_or(lhs, rhs, loc);
    }

    throw SemanticError(loc, "invalid binary operator");
}

}  // namespace dsl::ir::detail
