#include "dsl/semantic/detail/semantic_analyzer.hpp"

#include <algorithm>
#include <sstream>
#include <utility>
#include <vector>

#include "dsl/ast/definitions/pattern_definition.hpp"
#include "dsl/ast/definitions/track_definition.hpp"
#include "dsl/ast/definitions/voice_definition.hpp"
#include "dsl/ast/expression.hpp"
#include "dsl/ast/statement.hpp"
#include "dsl/ast/statements/assign_statement.hpp"
#include "dsl/ast/statements/for_statement.hpp"
#include "dsl/ast/statements/if_statement.hpp"
#include "dsl/ast/statements/let_statement.hpp"
#include "dsl/ast/statements/loop_statement.hpp"
#include "dsl/ast/statements/play_statement.hpp"
#include "dsl/semantic/annotations.hpp"
#include "dsl/semantic/detail/scopes/scope_stack.hpp"
#include "dsl/semantic/detail/types/type_rules.hpp"
#include "dsl/semantic/symbol_table.hpp"
#include "dsl/utils/overloaded.hpp"

namespace dsl::semantic::detail {

namespace {

class Traversal {
   public:
    explicit Traversal(AnalysisResult& result) : result_(result), scopes_(result.symbols()) {}

    void run(const ast::Program& program) {
        scopes_.push_scope();
        visit_globals(program.globals);
        for (const auto& track : program.tracks) {
            visit_track(track);
        }
        scopes_.pop_scope();
    }

   private:
    void visit_globals(const std::vector<ast::GlobalItem>& globals) {
        collect_global_patterns(globals);

        for (const auto& item : globals) {
            std::visit(utils::overloaded{
                           [&](const ast::StatementPtr& statement) {
                               if (statement) {
                                   visit_statement(*statement);
                               }
                           },
                           [&](const ast::PatternDefinition& pattern) { visit_pattern(pattern); },
                       },
                       item);
        }
    }

    void visit_track(const ast::TrackDefinition& track) {
        if (track.name) {
            (void)scopes_.add_symbol(*track.name, SymbolKind::Track, Type{TypeKind::Void}, track.location, &track);
        }

        scopes_.push_scope();
        collect_track_patterns(track.body);

        for (const auto& item : track.body) {
            std::visit(utils::overloaded{
                           [&](const ast::StatementPtr& statement) {
                               if (statement) {
                                   visit_statement(*statement);
                               }
                           },
                           [&](const ast::PatternDefinition& pattern) { visit_pattern(pattern); },
                           [&](const ast::VoiceDefinition& voice) { visit_voice(voice); },
                       },
                       item);
        }
        scopes_.pop_scope();
    }

    void visit_voice(const ast::VoiceDefinition& voice) {
        if (voice.from_expression) {
            const Type from_type = visit_expression(**voice.from_expression);
            if (is_known(from_type) && !is_numeric(from_type)) {
                diagnose(voice.location, "voice 'from' expression must be numeric");
            }
        }

        scopes_.push_scope();
        collect_voice_patterns(voice.body);

        for (const auto& item : voice.body) {
            std::visit(utils::overloaded{
                           [&](const ast::StatementPtr& statement) {
                               if (statement) {
                                   visit_statement(*statement);
                               }
                           },
                           [&](const ast::PatternDefinition& pattern) { visit_pattern(pattern); },
                       },
                       item);
        }
        scopes_.pop_scope();
    }

    void visit_pattern(const ast::PatternDefinition& pattern) {
        scopes_.push_scope();
        for (const auto& param : pattern.params) {
            (void)scopes_.add_symbol(param, SymbolKind::Parameter, Type{TypeKind::Unknown}, pattern.location, &pattern);
        }
        visit_block(pattern.body);
        scopes_.pop_scope();
    }

    void visit_block(const ast::Block& block) {
        for (const auto& statement : block) {
            if (statement) {
                visit_statement(*statement);
            }
        }
    }

    void visit_statement(const ast::Statement& statement) {
        std::visit(
            utils::overloaded{
                [&](const ast::AssignStatement& assign) {
                    const auto* symbol =
                        scopes_.find_visible(assign.name, {SymbolKind::Variable, SymbolKind::Parameter});
                    const SymbolId symbol_id = symbol ? symbol->id : INVALID_SYMBOL_ID;
                    if (!symbol) {
                        diagnose(statement.location, "assignment to undeclared variable '" + assign.name + "'");
                    }
                    const Type value_type = visit_expression(*assign.value);
                    if (symbol_id != INVALID_SYMBOL_ID) {
                        result_.symbols().set_symbol_type(symbol_id, value_type);
                    }
                },
                [&](const ast::ForStatement& for_stmt) {
                    scopes_.push_scope();
                    if (for_stmt.init) {
                        visit_statement(*for_stmt.init);
                    }
                    if (for_stmt.condition) {
                        const Type condition_type = visit_expression(*for_stmt.condition);
                        if (is_known(condition_type) && !is_boolean(condition_type)) {
                            diagnose(for_stmt.condition->location, "for condition must be a boolean");
                        }
                    }
                    visit_block(for_stmt.body);
                    if (for_stmt.step) {
                        visit_statement(*for_stmt.step);
                    }
                    scopes_.pop_scope();
                },
                [&](const ast::IfStatement& if_stmt) {
                    const Type condition_type = visit_expression(*if_stmt.condition);
                    if (is_known(condition_type) && !is_boolean(condition_type)) {
                        diagnose(if_stmt.condition->location, "if condition must be a boolean");
                    }
                    visit_block(if_stmt.then_branch);
                    if (if_stmt.else_branch) {
                        visit_block(*if_stmt.else_branch);
                    }
                },
                [&](const ast::LetStatement& let) {
                    const Type value_type = visit_expression(*let.value);
                    (void)scopes_.add_symbol(let.name, SymbolKind::Variable, value_type, statement.location, &let);
                },
                [&](const ast::LoopStatement& loop) {
                    const Type count_type = visit_expression(*loop.count);
                    if (is_known(count_type) && !is_integral(count_type)) {
                        diagnose(loop.count->location, "loop count must be an integer");
                    }
                    visit_block(loop.body);
                },
                [&](const ast::PlayStatement& play) { visit_play_target(play.target); },
            },
            statement.kind);
    }

    void collect_global_patterns(const std::vector<ast::GlobalItem>& globals) {
        for (const auto& item : globals) {
            if (const auto* pattern = std::get_if<ast::PatternDefinition>(&item)) {
                add_pattern_symbol(*pattern);
            }
        }
    }

    void collect_track_patterns(const std::vector<ast::TrackItem>& items) {
        for (const auto& item : items) {
            if (const auto* pattern = std::get_if<ast::PatternDefinition>(&item)) {
                add_pattern_symbol(*pattern);
            }
        }
    }

    void collect_voice_patterns(const std::vector<ast::VoiceItem>& items) {
        for (const auto& item : items) {
            if (const auto* pattern = std::get_if<ast::PatternDefinition>(&item)) {
                add_pattern_symbol(*pattern);
            }
        }
    }

    void add_pattern_symbol(const ast::PatternDefinition& pattern) {
        (void)
            scopes_.add_symbol(pattern.name, SymbolKind::Pattern, Type{TypeKind::Sequence}, pattern.location, &pattern);
    }

    void visit_play_target(const ast::PlayTarget& target) {
        std::visit(utils::overloaded{
                       [&](const ast::ExpressionPtr& expression) {
                           if (expression) {
                               visit_expression(*expression);
                           }
                       },
                       [](const auto&) {},
                   },
                   target.source);

        if (target.duration) {
            const Type duration_type = visit_expression(*target.duration);
            if (is_known(duration_type) && !is_numeric(duration_type)) {
                diagnose(target.duration->location, "play duration must be a number");
            }
        }
        if (target.from_offset) {
            const Type from_type = visit_expression(*target.from_offset);
            if (is_known(from_type) && !is_numeric(from_type)) {
                diagnose(target.from_offset->location, "from offset must be a number");
            }
        }
    }

    Type visit_expression(const ast::Expression& expression) {
        Type expression_type = std::visit(
            utils::overloaded{
                [&](const ast::IntLiteralExpression&) { return Type{TypeKind::Int}; },
                [&](const ast::FloatLiteralExpression&) { return Type{TypeKind::Double}; },
                [&](const ast::BoolLiteralExpression&) { return Type{TypeKind::Bool}; },
                [&](const ast::NoteLiteralExpression&) { return Type{TypeKind::Note}; },
                [&](const ast::RestLiteralExpression&) { return Type{TypeKind::Rest}; },
                [&](const ast::IdentifierExpression& identifier) { return visit_identifier(expression, identifier); },
                [&](const ast::UnaryExpression& unary) { return visit_unary(unary, expression.location); },
                [&](const ast::BinaryExpression& binary) { return visit_binary(binary, expression.location); },
                [&](const ast::TernaryExpression& ternary) { return visit_ternary(ternary, expression.location); },
                [&](const ast::ParenthesisedExpression& paren) { return visit_expression(*paren.inner); },
                [&](const ast::SequenceExpression& sequence) { return visit_sequence(sequence); },
                [&](const ast::ChordExpression& chord) { return visit_chord(chord, expression.location); },
                [&](const ast::CallExpression& call) { return visit_call(call, expression.location); },
            },
            expression.kind);

        result_.annotations().set_expression_type(expression, expression_type);
        return expression_type;
    }

    Type visit_identifier(const ast::Expression& expression, const ast::IdentifierExpression& identifier) {
        if (const auto* symbol = scopes_.find_visible(identifier.name, {SymbolKind::Variable, SymbolKind::Parameter})) {
            result_.annotations().set_resolved_symbol(expression, symbol->id);
            return symbol->type;
        }

        diagnose(expression.location, "undefined variable '" + identifier.name + "'");
        return Type{TypeKind::Unknown};
    }

    Type visit_unary(const ast::UnaryExpression& unary, const Location& location) {
        const Type operand_type = visit_expression(*unary.operand);

        switch (unary.operation) {
            case ast::UnaryOperator::Negative:
                if (is_known(operand_type) && !is_numeric(operand_type)) {
                    diagnose(location, "unary '-' requires a numeric operand");
                    return Type{TypeKind::Unknown};
                }
                return is_numeric(operand_type) ? operand_type : Type{TypeKind::Unknown};
            case ast::UnaryOperator::Not:
                if (is_known(operand_type) && !is_boolean(operand_type)) {
                    diagnose(location, "unary '!' requires a boolean operand");
                    return Type{TypeKind::Unknown};
                }
                return is_boolean(operand_type) ? Type{TypeKind::Bool} : Type{TypeKind::Unknown};
        }

        diagnose(location, "invalid unary operator");
        return Type{TypeKind::Unknown};
    }

    Type visit_binary(const ast::BinaryExpression& binary, const Location& location) {
        const Type left_type = visit_expression(*binary.left);
        const Type right_type = visit_expression(*binary.right);

        validate_binary_operands(binary.operation, left_type, right_type, location);
        return binary_result_type(binary.operation, left_type, right_type);
    }

    void validate_binary_operands(const ast::BinaryOperator op,
                                  const Type left_type,
                                  const Type right_type,
                                  const Location& location) {
        using Op = ast::BinaryOperator;

        switch (op) {
            case Op::Add:
            case Op::Subtract:
            case Op::Multiply:
            case Op::Divide:
            case Op::Less:
            case Op::Greater:
            case Op::LessOrEqual:
            case Op::GreaterOrEqual:
                validate_numeric_operand(left_type, "left", location);
                validate_numeric_operand(right_type, "right", location);
                return;
            case Op::Modulo:
                if ((is_known(left_type) && !is_integral(left_type)) ||
                    (is_known(right_type) && !is_integral(right_type))) {
                    diagnose(location, "modulo requires integer operands");
                }
                return;
            case Op::Equals:
            case Op::NotEquals:
                if (!is_known(left_type) || !is_known(right_type)) {
                    return;
                }
                if ((is_numeric(left_type) && is_numeric(right_type)) ||
                    (is_boolean(left_type) && is_boolean(right_type))) {
                    return;
                }
                diagnose(location, "'==' requires numeric or boolean operands");
                return;
            case Op::And:
            case Op::Or:
                if ((is_known(left_type) && !is_boolean(left_type)) ||
                    (is_known(right_type) && !is_boolean(right_type))) {
                    diagnose(location, "'&&' requires boolean operands");
                }
                return;
        }

        diagnose(location, "invalid binary operator");
    }

    void validate_numeric_operand(const Type type, const char* side, const Location& location) {
        if (is_known(type) && !is_numeric(type)) {
            diagnose(location, std::string(side) + " operand must be numeric");
        }
    }

    Type visit_ternary(const ast::TernaryExpression& ternary, const Location& location) {
        const Type condition_type = visit_expression(*ternary.condition);
        if (is_known(condition_type) && !is_boolean(condition_type)) {
            diagnose(location, "ternary condition must be a boolean expression");
        }

        const Type then_type = visit_expression(*ternary.then_expression);
        const Type else_type = visit_expression(*ternary.else_expression);
        if (is_known(then_type) && is_known(else_type) && !same_known_type(then_type, else_type)) {
            diagnose(location, "ternary branches must evaluate to the same type");
            return Type{TypeKind::Unknown};
        }

        return same_known_type(then_type, else_type) ? then_type : Type{TypeKind::Unknown};
    }

    Type visit_sequence(const ast::SequenceExpression& sequence) {
        for (const auto& [value, duration] : sequence.items) {
            (void)visit_expression(*value);
            if (duration) {
                const Type duration_type = visit_expression(*duration);
                if (is_known(duration_type) && !is_numeric(duration_type)) {
                    diagnose(duration->location, "sequence item duration must be numeric");
                }
            }
        }

        return Type{TypeKind::Sequence};
    }

    Type visit_chord(const ast::ChordExpression& chord, const Location& location) {
        for (const auto& [value, duration] : chord.notes) {
            const Type value_type = visit_expression(*value);
            if (is_known(value_type) && !is_note(value_type)) {
                diagnose(location, "chord members must be notes");
            }

            if (duration) {
                const Type duration_type = visit_expression(*duration);
                if (is_known(duration_type) && !is_numeric(duration_type)) {
                    diagnose(duration->location, "chord note duration must be numeric");
                }
            }
        }

        return Type{TypeKind::Chord};
    }

    Type visit_call(const ast::CallExpression& call, const Location& location) {
        std::vector<Type> argument_types;
        argument_types.reserve(call.arguments.size());
        for (const auto& arg : call.arguments) {
            argument_types.push_back(visit_expression(*arg));
        }

        validate_call(call, location, argument_types);
        return Type{TypeKind::Unknown};
    }

    void validate_call(const ast::CallExpression& call,
                       const Location& location,
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

    void validate_pattern_instantiation(const ast::PatternDefinition& pattern,
                                        const std::vector<Type>& argument_types) {
        if (is_pattern_active(pattern)) {
            return;
        }

        active_patterns_.push_back(&pattern);
        scopes_.push_scope();
        for (std::size_t i = 0; i < pattern.params.size(); ++i) {
            (void)scopes_.add_symbol(pattern.params[i],
                                     SymbolKind::Parameter,
                                     argument_types[i],
                                     pattern.location,
                                     &pattern);
        }
        visit_block(pattern.body);
        scopes_.pop_scope();
        active_patterns_.pop_back();
    }

    bool is_pattern_active(const ast::PatternDefinition& pattern) const {
        return std::ranges::find(active_patterns_, &pattern) != active_patterns_.end();
    }

    void diagnose(Location location, std::string message) {
        result_.diagnostics().push_back(Diagnostic{
            .severity = DiagnosticSeverity::Error,
            .location = std::move(location),
            .message = std::move(message),
        });
    }

    AnalysisResult& result_;
    ScopeStack scopes_;
    std::vector<const ast::PatternDefinition*> active_patterns_;
};

}  // namespace

SemanticAnalyzer::SemanticAnalyzer(const ast::Program& program) : program_(program) {}

AnalysisResult SemanticAnalyzer::analyze() {
    AnalysisResult result(program_);
    Traversal(result).run(program_);
    return result;
}

}  // namespace dsl::semantic::detail
