#include <sstream>
#include <vector>

#include "dsl/common/ast/definitions.hpp"
#include "dsl/semantic/detail/traversal.hpp"
#include "dsl/semantic/detail/types/type_rules.hpp"

namespace dsl::semantic::detail {

namespace {

class ActivePatternGuard {
   public:
    ActivePatternGuard(std::vector<const ast::PatternDefinition*> patterns, const ast::PatternDefinition* pattern)
        : patterns_(patterns) {
        patterns_.push_back(pattern);
    }

    ActivePatternGuard(const ActivePatternGuard&) = delete;
    ActivePatternGuard& operator=(const ActivePatternGuard&) = delete;

    ~ActivePatternGuard() { patterns_.pop_back(); }

   private:
    std::vector<const ast::PatternDefinition*>& patterns_;
};

}  // namespace

void Traversal::validate_binary_operands(const ast::BinaryOperator op,
                                         const Type left_type,
                                         const Type right_type,
                                         const source::Location& location) const {
    using Op = ast::BinaryOperator;

    switch (op) {
        case Op::Add:
        case Op::Subtract:
        case Op::Multiply:
        case Op::Divide:
        case Op::Less:
        case Op::Greater:
        case Op::LessOrEqual:
        case Op::GreaterOrEqual: {
            validate_numeric_operand(left_type, "left", location);
            validate_numeric_operand(right_type, "right", location);

            return;
        }
        case Op::Modulo: {
            if ((is_known(left_type) && !is_integral(left_type)) ||
                (is_known(right_type) && !is_integral(right_type))) {
                diagnose(location, "modulo requires integer operands");
            }

            return;
        }
        case Op::Equals:
        case Op::NotEquals: {
            if (!is_known(left_type) || !is_known(right_type)) {
                return;
            }

            if ((is_numeric(left_type) && is_numeric(right_type)) ||
                (is_boolean(left_type) && is_boolean(right_type))) {
                return;
            }

            diagnose(location, "'==' requires numeric or boolean operands");
            return;
        }
        case Op::And:
        case Op::Or: {
            if ((is_known(left_type) && !is_boolean(left_type)) || (is_known(right_type) && !is_boolean(right_type))) {
                diagnose(location, "'&&' requires boolean operands");
            }

            return;
        }
    }

    diagnose(location, "invalid binary operator");
}

void Traversal::validate_numeric_operand(const Type type, const char* side, const source::Location& location) const {
    if (is_known(type) && !is_numeric(type)) {
        diagnose(location, std::string(side) + " operand must be numeric");
    }
}

void Traversal::validate_call(const ast::PatternCallExpression& call,
                              const source::Location& location,
                              const std::vector<Type>& argument_types) {
    const auto* symbol = scopes_.find_pattern_visible_by_arity(call.callee, call.arguments.size());
    if (!symbol) {
        // Distinguish "pattern doesn't exist" from "no overload with this arity"
        const auto* any_overload = scopes_.find_visible(call.callee, {SymbolKind::Pattern});
        if (!any_overload) {
            diagnose(location, "undefined pattern '" + call.callee + "'");
        } else {
            std::stringstream ss;
            ss << "no overload of pattern '" << call.callee << "' takes " << call.arguments.size() << " argument(s)";
            diagnose(location, ss.str());
        }
        return;
    }

    const auto* pattern = static_cast<const ast::PatternDefinition*>(symbol->declaration);
    if (!pattern) {
        diagnose(location, "pattern '" + call.callee + "' is missing declaration metadata");
        return;
    }

    validate_pattern_instantiation(*pattern, argument_types);
}

void Traversal::validate_pattern_instantiation(const ast::PatternDefinition& pattern,
                                               const std::vector<Type>& argument_types) {
    if (is_pattern_active(pattern)) {
        return;
    }

    ActivePatternGuard pattern_guard(active_patterns_, &pattern);
    ScopeStack::Guard scope_guard(scopes_);

    const ScopeId prev_boundary = writable_boundary_;
    // Patterns called via validate_call inherit the enclosing boundary.
    // Only establish a new boundary if none exists (global pattern).
    if (writable_boundary_ == INVALID_SCOPE_ID) {
        writable_boundary_ = scopes_.current_scope();
    }

    for (std::size_t i = 0; i < pattern.params.size(); ++i) {
        scopes_.add_symbol(pattern.params[i], SymbolKind::Parameter, argument_types[i], pattern.location, &pattern);
    }

    const bool previous_skip = skip_symbol_annotation_;
    skip_symbol_annotation_ = true;
    visit_block(pattern.body);
    skip_symbol_annotation_ = previous_skip;

    writable_boundary_ = prev_boundary;
}

}  // namespace dsl::semantic::detail
