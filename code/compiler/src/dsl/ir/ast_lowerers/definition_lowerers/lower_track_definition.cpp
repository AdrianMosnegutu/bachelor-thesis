#include <algorithm>

#include "dsl/ir/ast_lowerer.hpp"
#include "dsl/ir/lowerer_context.hpp"
#include "dsl/ir/track.hpp"
#include "dsl/utils/overloaded.hpp"

namespace dsl::ir {

Track lower_track_definition(const ast::TrackDefinition& track, LowererContext& ctx) {
    Track out;
    out.name = track.name;
    if (track.instrument) {
        out.instrument = *track.instrument;
    }

    // Collect local pattern definitions (shadow globals).
    ctx.collect_track_patterns(track.body);

    ctx.push_scope();
    double cursor = 0.0;

    for (const auto& item : track.body) {
        std::visit(utils::overloaded{
                       [&](const ast::StatementPtr& ptr) {
                           auto events = lower_statement(*ptr, ctx, cursor);
                           out.events.insert(out.events.end(), events.begin(), events.end());
                       },
                       [&](const ast::VoiceDefinition& def) {
                           auto events = lower_voice_definition(def, ctx, cursor);
                           out.events.insert(out.events.end(), events.begin(), events.end());
                       },
                       [&](const auto&) {}  // PatternDefinition items are skipped here (already collected above).
                   },
                   item);
    }

    ctx.pop_scope();
    ctx.erase_track_patterns(track.body);

    std::ranges::stable_sort(out.events,
                             [](const NoteEvent& a, const NoteEvent& b) { return a.start_beat < b.start_beat; });
    return out;
}

}  // namespace dsl::ir
