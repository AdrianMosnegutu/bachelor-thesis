#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "dsl/ast/program.hpp"
#include "dsl/ast/statements.hpp"
#include "dsl/diagnostics/diagnostics_engine.hpp"
#include "dsl/ir/note_event.hpp"
#include "dsl/ir/values.hpp"
#include "dsl/source/location.hpp"

namespace dsl::lowerer::detail {

class LoweringFailure final : public std::runtime_error {
   public:
    LoweringFailure(const source::Location& loc, const std::string& msg);
};

class LowererContext {
   public:
    static constexpr int MAX_ITERATIONS = 10000;

    using Scope = std::unordered_map<std::string, ir::Value>;
    using PatternMap = std::unordered_map<std::string, const ast::PatternDefinition*>;
    using BlockExecutor = std::function<std::vector<ir::NoteEvent>(const ast::Block&, double&)>;

    explicit LowererContext(DiagnosticsEngine& diagnostics);

    BlockExecutor execute_block;

    void push_scope();
    void pop_scope();
    void bind(const std::string& name, ir::Value val);
    [[nodiscard]] const ir::Value& lookup(const std::string& name, const source::Location& loc) const;
    void assign(const std::string& name, ir::Value val, const source::Location& loc);

    void collect_patterns(const std::vector<ast::GlobalItem>& globals);
    void collect_track_patterns(const std::vector<ast::TrackItem>& items);
    void erase_track_patterns(const std::vector<ast::TrackItem>& items);
    void collect_voice_patterns(const std::vector<ast::VoiceItem>& items);
    void erase_voice_patterns(const std::vector<ast::VoiceItem>& items);

    [[nodiscard]] const ast::PatternDefinition* find_pattern(const std::string& name) const;

    void report_lowering_error(std::string message);

   private:
    DiagnosticsEngine& diagnostics_;
    std::vector<Scope> scope_stack_;
    PatternMap patterns_;
};

class LowererScopeGuard {
   public:
    explicit LowererScopeGuard(LowererContext& ctx);
    ~LowererScopeGuard();

    LowererScopeGuard(const LowererScopeGuard&) = delete;
    LowererScopeGuard& operator=(const LowererScopeGuard&) = delete;

   private:
    LowererContext& ctx_;
};

}  // namespace dsl::lowerer::detail
