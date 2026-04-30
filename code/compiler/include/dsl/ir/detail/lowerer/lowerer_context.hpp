#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dsl/core/ast/program.hpp"
#include "dsl/core/ast/statement.hpp"
#include "dsl/core/location.hpp"
#include "dsl/ir/detail/value.hpp"
#include "dsl/ir/program.hpp"

namespace dsl::ir::detail {

class LowererContext {
   public:
    static constexpr int MAX_ITERATIONS = 10000;

    using Scope = std::unordered_map<std::string, Value>;
    using PatternMap = std::unordered_map<std::string, const ast::PatternDefinition*>;
    using BlockExecutor = std::function<std::vector<NoteEvent>(const ast::Block&, double&)>;

    BlockExecutor execute_block;

    void push_scope();
    void pop_scope();
    void bind(const std::string& name, Value val);
    [[nodiscard]] const Value& lookup(const std::string& name, const Location& loc) const;
    void assign(const std::string& name, Value val, const Location& loc);

    void collect_patterns(const std::vector<ast::GlobalItem>& globals);
    void collect_track_patterns(const std::vector<ast::TrackItem>& items);
    void erase_track_patterns(const std::vector<ast::TrackItem>& items);
    void collect_voice_patterns(const std::vector<ast::VoiceItem>& items);
    void erase_voice_patterns(const std::vector<ast::VoiceItem>& items);

    [[nodiscard]] const ast::PatternDefinition* find_pattern(const std::string& name) const;

   private:
    std::vector<Scope> scope_stack_;
    PatternMap patterns_;
};

}  // namespace dsl::ir::detail
