#pragma once

#include "dsl/common/ast/statements.hpp"
#include "dsl/common/diagnostics/diagnostics_engine.hpp"
#include "dsl/common/source/location.hpp"
#include "dsl/semantic/analysis_result.hpp"
#include "dsl/semantic/detail/scopes/scope_stack.hpp"

namespace dsl::semantic::detail {

class Traversal {
   public:
    Traversal(AnalysisResult& result, DiagnosticsEngine& diagnostics);

    void run(const ast::Program& program);

   private:
    AnalysisResult& result_;
    DiagnosticsEngine& diagnostics_;
    ScopeStack scopes_;
    std::vector<const ast::PatternDefinition*> active_patterns_;

    void add_pattern_symbol(const ast::PatternDefinition& pattern) const;
    [[nodiscard]] bool is_pattern_active(const ast::PatternDefinition& pattern) const;

    void diagnose(const source::Location& location, std::string message) const;

    void visit_globals(const std::vector<ast::GlobalItem>& globals);
    void visit_track(const ast::TrackDefinition& track);
    void visit_voice(const ast::VoiceDefinition& voice);
    void visit_pattern(const ast::PatternDefinition& pattern);

    void visit_block(const ast::Block& block);
    void visit_statement(const ast::Statement& statement);
    void visit_assign_statement(const ast::AssignStatement& assign, const source::Location& location);
    void visit_if_statement(const ast::IfStatement& if_stmt, const source::Location& location);
    void visit_for_statement(const ast::ForStatement& for_stmt, const source::Location& location);
    void visit_loop_statement(const ast::LoopStatement& loop, const source::Location& location);
    void visit_let_statement(const ast::LetStatement& let, const source::Location& location);
    void visit_play_target(const ast::PlayTarget& target);

    Type visit_expression(const ast::Expression& expression);
    Type visit_identifier(const ast::Expression& expression, const ast::IdentifierExpression& identifier) const;
    Type visit_unary(const ast::UnaryExpression& unary, const source::Location& location);
    Type visit_binary(const ast::BinaryExpression& binary, const source::Location& location);
    Type visit_ternary(const ast::TernaryExpression& ternary, const source::Location& location);
    Type visit_sequence(const ast::SequenceExpression& sequence);
    Type visit_chord(const ast::ChordExpression& chord, const source::Location& location);
    Type visit_call(const ast::PatternCallExpression& call, const source::Location& location);

    void validate_binary_operands(const ast::BinaryOperator op,
                                  const Type left_type,
                                  const Type right_type,
                                  const source::Location& location) const;
    void validate_numeric_operand(const Type type, const char* side, const source::Location& location) const;
    void validate_call(const ast::PatternCallExpression& call,
                       const source::Location& location,
                       const std::vector<Type>& argument_types);
    void validate_pattern_instantiation(const ast::PatternDefinition& pattern, const std::vector<Type>& argument_types);

    void collect_global_patterns(const std::vector<ast::GlobalItem>& globals) const;
    void collect_track_patterns(const std::vector<ast::TrackItem>& items) const;
    void collect_voice_patterns(const std::vector<ast::VoiceItem>& items) const;
};

}  // namespace dsl::semantic::detail
