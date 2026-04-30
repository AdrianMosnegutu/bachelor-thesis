#include "type_rules.hpp"

namespace dsl::semantic::detail {

bool is_known(const Type type) { return type.kind != TypeKind::Unknown; }

bool is_numeric(const Type type) { return type.kind == TypeKind::Int || type.kind == TypeKind::Double; }

bool is_integral(const Type type) { return type.kind == TypeKind::Int; }

bool is_boolean(const Type type) { return type.kind == TypeKind::Bool; }

bool is_note(const Type type) { return type.kind == TypeKind::Note; }

bool same_known_type(const Type left, const Type right) { return is_known(left) && is_known(right) && left == right; }

Type numeric_result(const Type left, const Type right) {
    if (!is_numeric(left) || !is_numeric(right)) {
        return Type{TypeKind::Unknown};
    }

    return Type{left.kind == TypeKind::Double || right.kind == TypeKind::Double ? TypeKind::Double : TypeKind::Int};
}

Type binary_result_type(const ast::BinaryOperator op, const Type left, const Type right) {
    using Op = ast::BinaryOperator;

    switch (op) {
        case Op::Add:
        case Op::Subtract:
        case Op::Multiply:
            return numeric_result(left, right);
        case Op::Divide:
            return is_numeric(left) && is_numeric(right) ? Type{TypeKind::Double} : Type{TypeKind::Unknown};
        case Op::Modulo:
            return is_integral(left) && is_integral(right) ? Type{TypeKind::Int} : Type{TypeKind::Unknown};
        case Op::Equals:
        case Op::NotEquals:
        case Op::Less:
        case Op::Greater:
        case Op::LessOrEqual:
        case Op::GreaterOrEqual:
        case Op::And:
        case Op::Or:
            return Type{TypeKind::Bool};
    }

    return Type{TypeKind::Unknown};
}

}  // namespace dsl::semantic::detail
