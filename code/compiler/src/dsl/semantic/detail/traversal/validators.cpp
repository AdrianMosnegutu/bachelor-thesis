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
    const auto* symbol = scopes_.find_visible(call.callee, {SymbolKind::Pattern});
    if (!symbol) {
        diagnose(location, "undefined pattern '" + call.callee + "'");
        return;
    }

    const auto* pattern = static_cast<const ast::PatternDefinition*>(symbol->declaration);
    if (!pattern) {
        diagnose(location, "pattern '" + call.callee + "' is missing declaration metadata");
        return;
    }

    if (call.arguments.size() != pattern->params.size()) {
        std::stringstream ss;

        ss << "pattern '" << call.callee << "' ";
        ss << "expects " << pattern->params.size() << " argument(s), ";
        ss << "got " << call.arguments.size();

        diagnose(location, ss.str());
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

    for (std::size_t i = 0; i < pattern.params.size(); ++i) {
        scopes_.add_symbol(pattern.params[i], SymbolKind::Parameter, argument_types[i], pattern.location, &pattern);
    }

    visit_block(pattern.body);
}

}  // namespace dsl::semantic::detail
