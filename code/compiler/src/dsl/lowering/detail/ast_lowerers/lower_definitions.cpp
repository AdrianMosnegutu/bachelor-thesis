#include <algorithm>

#include "dsl/common/utils/overloaded.hpp"
#include "dsl/lowering/detail/ast_lowerer.hpp"
#include "dsl/lowering/detail/expression_evaluator.hpp"

namespace dsl::lowering::detail {

ir::Track lower_track_definition(const ast::TrackDefinition& track, LowererContext& ctx) {
    ir::Track out;
    out.name = track.name;
    if (track.instrument) {
        out.instrument = *track.instrument;
    }

    // Collect local pattern definitions (shadow globals).
    ctx.collect_track_patterns(track.body);

    LowererScopeGuard scope(ctx);
    double cursor = 0.0;

    for (const auto& item : track.body) {
        try {
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
        } catch (const LoweringFailure& error) {
            ctx.report_lowering_error(error.what());
        }
    }

    ctx.erase_track_patterns(track.body);

    std::ranges::stable_sort(out.events, [](const ir::NoteEvent& a, const ir::NoteEvent& b) {
        return a.start_beat < b.start_beat;
    });

    return out;
}

ir::NoteEvents lower_voice_definition(const ast::VoiceDefinition& voice,
                                      LowererContext& ctx,
                                      const double outer_cursor) {
    double voice_cursor = outer_cursor;

    if (voice.from_expression) {
        const auto [kind] = evaluate_expression(**voice.from_expression, ctx);

        voice_cursor = std::visit(utils::overloaded{[](const int number) { return static_cast<double>(number); },
                                                    [](const double number) { return number; },
                                                    [&voice](const auto&) -> double {
                                                        throw LoweringFailure(
                                                            voice.location,
                                                            "lowering reached voice with non-numeric from expression");
                                                    }},
                                  kind);
    }

    ctx.collect_voice_patterns(voice.body);
    LowererScopeGuard scope(ctx);

    std::vector<ir::NoteEvent> events;
    for (const auto& item : voice.body) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            try {
                auto evs = lower_statement(**stmt_ptr, ctx, voice_cursor);
                events.insert(events.end(), evs.begin(), evs.end());
            } catch (const LoweringFailure& error) {
                ctx.report_lowering_error(error.what());
            }
        }
        // PatternDefinition items already registered — skip execution.
    }

    ctx.erase_voice_patterns(voice.body);

    // outer_cursor is intentionally not modified — voice runs in parallel.
    return events;
}

void lower_header(const ast::Header& header, ir::Program& out) {
    if (header.tempo) {
        out.tempo_bpm = header.tempo->beats_per_minute;
    }

    if (header.signature) {
        out.time_sig_numerator = header.signature->beats;
        out.time_sig_denominator = header.signature->unit;
    }
}

}  // namespace dsl::lowering::detail
