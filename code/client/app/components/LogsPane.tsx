"use client";

import type { LogEntry } from "../types";

const BADGE_COLORS: Record<string, string> = {
  lexical: "bg-yellow-900/60 text-yellow-300 border-yellow-700",
  syntax: "bg-orange-900/60 text-orange-300 border-orange-700",
  semantic: "bg-red-900/60 text-red-300 border-red-700",
  internal: "bg-zinc-700/60 text-zinc-300 border-zinc-600",
};

function fmt(d: Date) {
  return d.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
}

interface LogsPaneProps {
  log: LogEntry | null;
  onClear: () => void;
}

export default function LogsPane({ log, onClear }: LogsPaneProps) {
  return (
    <div className="flex flex-col h-full bg-panel">
      <div className="shrink-0 flex items-center justify-between px-3 py-1 border-b border-border bg-toolbar">
        <div className="flex items-center gap-2">
          <span className="text-xs text-zinc-600 dark:text-zinc-400 font-mono uppercase tracking-wider">
            logs
          </span>
          <span className="text-[10px] text-zinc-400 dark:text-zinc-500 font-mono">
            ⌘J / Ctrl+J
          </span>
        </div>
        <button
          onClick={onClear}
          disabled={!log}
          className="text-xs text-zinc-600 dark:text-zinc-400 hover:text-zinc-900 dark:hover:text-zinc-200 disabled:opacity-30 disabled:cursor-not-allowed transition-colors font-medium"
        >
          Clear
        </button>
      </div>

      <div className="flex-1 overflow-y-auto px-3 py-2 font-mono text-sm">
        {!log ? (
          <span className="text-zinc-500">No output yet.</span>
        ) : log.kind === "success" ? (
          <div className="flex items-center gap-2 text-emerald-600 dark:text-emerald-400">
            <span>✓</span>
            <span>Compiled successfully</span>
            <span className="text-zinc-500 dark:text-zinc-400 text-xs ml-auto">{fmt(log.timestamp)}</span>
          </div>
        ) : (
          <div className="space-y-1">
            <div className="flex items-center gap-2 flex-wrap">
              <span
                className={`text-xs px-1.5 py-0.5 rounded border font-semibold uppercase tracking-wide ${BADGE_COLORS[log.type] ?? BADGE_COLORS.internal}`}
              >
                {log.type}
              </span>
              {log.line != null && (
                <span className="text-zinc-600 dark:text-zinc-300 text-xs">
                  line {log.line}
                  {log.column != null ? `:${log.column}` : ""}
                </span>
              )}
              <span className="text-zinc-500 dark:text-zinc-400 text-xs ml-auto">{fmt(log.timestamp)}</span>
            </div>
            <p className="text-red-600 dark:text-red-300 break-all font-medium">{log.message}</p>
          </div>
        )}
      </div>

    </div>
  );
}
