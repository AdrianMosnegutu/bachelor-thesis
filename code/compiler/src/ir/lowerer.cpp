#include "dsl/ir/lowerer.hpp"

#include <algorithm>

#include "dsl/ast/decl.hpp"
#include "dsl/ast/stmt.hpp"
#include "dsl/errors/semantic_error.hpp"
#include "dsl/ir/lowerer_context.hpp"

namespace dsl::ir {

using errors::SemanticError;

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------

ProgramIR Lowerer::lower(const ast::Program& program) {
    ProgramIR out;
    lower_header(program.header, out);

    LowererContext ctx;
    ctx.collect_patterns(program.globals);
    ctx.execute_block = [this, &ctx](const ast::Block& b, double& cur) { return lower_block(b, ctx, cur); };

    // Lower global let bindings (patterns already collected above).
    ctx.push_scope();
    for (const auto& item : program.globals) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            if (const auto* let = dynamic_cast<const ast::LetStatement*>(stmt_ptr->get())) {
                lower_let(*let, ctx);
            }
        }
    }

    for (const auto& track : program.tracks) {
        out.tracks.push_back(lower_track(track, ctx));
    }
    ctx.pop_scope();
    return out;
}

// ---------------------------------------------------------------------------
// Header lowering
// ---------------------------------------------------------------------------

void Lowerer::lower_header(const ast::Header& header, ProgramIR& out) {
    if (header.tempo) out.tempo_bpm = header.tempo->bpm;
    if (header.signature) {
        out.time_sig_numerator = header.signature->beats;
        out.time_sig_denominator = header.signature->unit;
    }
    if (header.key) {
        out.key = KeySignature{header.key->pitch, header.key->mode};
    }
}

// ---------------------------------------------------------------------------
// Track lowering
// ---------------------------------------------------------------------------

TrackIR Lowerer::lower_track(const ast::TrackDeclaration& track, LowererContext& ctx) {
    TrackIR out;
    out.name = track.name;
    if (track.instrument) {
        out.instrument = *track.instrument;
    }

    // Collect local pattern definitions (shadow globals).
    ctx.collect_track_patterns(track.body);

    ctx.push_scope();
    double cursor = 0.0;

    for (const auto& item : track.body) {
        if (const auto* stmt_ptr = std::get_if<ast::StatementPtr>(&item)) {
            auto events = lower_stmt(**stmt_ptr, ctx, cursor);
            out.events.insert(out.events.end(), events.begin(), events.end());
        } else if (const auto* voice_ptr = std::get_if<ast::VoiceDeclaration>(&item)) {
            auto events = lower_voice(*voice_ptr, ctx, cursor);
            out.events.insert(out.events.end(), events.begin(), events.end());
        }
        // PatternDefinition items are skipped here (already collected above).
    }

    ctx.pop_scope();
    ctx.erase_track_patterns(track.body);

    std::ranges::stable_sort(out.events,
                             [](const NoteEvent& a, const NoteEvent& b) { return a.start_beat < b.start_beat; });
    return out;
}

// ---------------------------------------------------------------------------
// Block / statement dispatch
// ---------------------------------------------------------------------------

std::vector<NoteEvent> Lowerer::lower_block(const ast::Block& block, LowererContext& ctx, double& cursor) {
    std::vector<NoteEvent> events;
    ctx.push_scope();
    for (const auto& stmt_ptr : block) {
        auto evs = lower_stmt(*stmt_ptr, ctx, cursor);
        events.insert(events.end(), evs.begin(), evs.end());
    }
    ctx.pop_scope();
    return events;
}

std::vector<NoteEvent> Lowerer::lower_stmt(const ast::Statement& stmt, LowererContext& ctx, double& cursor) {
    if (const auto* s = dynamic_cast<const ast::PlayStatement*>(&stmt)) {
        return lower_play(*s, ctx, cursor);
    }
    if (const auto* s = dynamic_cast<const ast::ForStatement*>(&stmt)) {
        return lower_for(*s, ctx, cursor);
    }
    if (const auto* s = dynamic_cast<const ast::LoopStatement*>(&stmt)) {
        return lower_loop(*s, ctx, cursor);
    }
    if (const auto* s = dynamic_cast<const ast::IfStatement*>(&stmt)) {
        return lower_if(*s, ctx, cursor);
    }
    if (const auto* s = dynamic_cast<const ast::LetStatement*>(&stmt)) {
        lower_let(*s, ctx);
        return {};
    }
    if (const auto* s = dynamic_cast<const ast::AssignStatement*>(&stmt)) {
        lower_assign(*s, ctx);
        return {};
    }
    throw SemanticError(stmt.loc, "unhandled statement type in lowerer");
}

}  // namespace dsl::ir
