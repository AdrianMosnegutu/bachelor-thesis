#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dsl/ast/program.hpp"
#include "dsl/ast/statements.hpp"
#include "dsl/ir/note_event.hpp"
#include "dsl/ir/values.hpp"
#include "dsl/source/location.hpp"

namespace dsl::lowerer::detail {

class LowererContext {
   public:
    static constexpr int MAX_ITERATIONS = 10000;

    using Scope = std::unordered_map<std::string, ir::Value>;
    using PatternMap = std::unordered_map<std::string, const ast::PatternDefinition*>;
    using BlockExecutor = std::function<std::vector<ir::NoteEvent>(const ast::Block&, double&)>;

    BlockExecutor execute_block;

    void push_scope();
    void pop_scope();
    void bind(const std::string& name, ir::Value val);
    [[nodiscard]] const ir::Value& lookup(const std::string& name, const Location& loc) const;
    void assign(const std::string& name, ir::Value val, const Location& loc);

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

}  // namespace dsl::lowerer::detail
