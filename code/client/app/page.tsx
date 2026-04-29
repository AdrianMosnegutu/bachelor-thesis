"use client";

import dynamic from "next/dynamic";
import { useCallback, useEffect, useRef, useState } from "react";
import { Panel, Group, usePanelRef } from "react-resizable-panels";

import { useMidi } from "./context/MidiContext";
import LogsPane from "./components/LogsPane";
import PlaybackBar from "./components/PlaybackBar";
import ResizeHandle from "./components/ResizeHandle";
import type { LogEntry } from "./types";
import type { DslEditorHandle } from "./components/DslEditor";

const DslEditor = dynamic(() => import("./components/DslEditor"), {
  ssr: false,
  loading: () => (
    <div className="flex flex-1 items-center justify-center text-zinc-400 font-mono text-sm">
      Loading editor…
    </div>
  ),
});

const PianoRoll = dynamic(() => import("./components/PianoRoll"), {
  ssr: false,
  loading: () => (
    <div className="flex flex-1 items-center justify-center text-zinc-400 font-mono text-xs bg-[#0d0d0f]">
      Loading piano roll…
    </div>
  ),
});

function Spinner() {
  return (
    <svg
      className="animate-spin w-3.5 h-3.5"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth={2.5}
    >
      <path
        strokeLinecap="round"
        d="M12 2a10 10 0 0 1 10 10"
        className="opacity-80"
      />
    </svg>
  );
}

export default function Home() {
  const { setMidiBytes } = useMidi();
  const [compiling, setCompiling] = useState(false);
  const [log, setLog] = useState<LogEntry | null>(null);
  const sourceRef = useRef<string>("");
  const editorRef = useRef<DslEditorHandle>(null);

  const handleEditorChange = useCallback((value: string) => {
    sourceRef.current = value;
  }, []);

  const handleCompile = useCallback(async () => {
    if (compiling) return;
    setCompiling(true);
    setLog(null);
    setMidiBytes(null);
    editorRef.current?.clearError();

    try {
      const res = await fetch("/api/compile", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ source: sourceRef.current }),
      });

      if (res.ok) {
        const buf = await res.arrayBuffer();
        setMidiBytes(new Uint8Array(buf));
        setLog({ kind: "success", timestamp: new Date() });
        editorRef.current?.blur();
      } else {
        const data = await res.json();
        const err = data?.error ?? {};
        setLog({
          kind: "error",
          type: err.type ?? "internal",
          message: err.message ?? "Unknown error",
          line: err.line,
          column: err.column,
          timestamp: new Date(),
        });

        if (err.line != null && err.column != null) {
          editorRef.current?.setError(err.line, err.column, err.message ?? "Error");
          editorRef.current?.jumpTo(err.line, err.column);
        }
      }
    } catch {
      setLog({
        kind: "error",
        type: "internal",
        message: "Network error: could not reach backend",
        timestamp: new Date(),
      });
    } finally {
      setCompiling(false);
    }
  }, [compiling, setMidiBytes]);

  const logsPanelRef = usePanelRef();

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Cmd/Ctrl + J to toggle logs
      if ((e.metaKey || e.ctrlKey) && e.key === "j") {
        e.preventDefault();
        const panel = logsPanelRef.current;
        if (panel) {
          if (panel.isCollapsed()) {
            panel.expand();
          } else {
            panel.collapse();
          }
        }
      }
    };

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [logsPanelRef]);

  return (
    <div className="flex-1 flex flex-col min-h-0">
      <Group orientation="horizontal" className="flex-1">
        {/* Left Pane: Editor & Logs */}
        <Panel defaultSize={60} minSize={30}>
          <Group orientation="vertical">
            <Panel defaultSize={70} minSize={20} className="flex flex-col min-h-0">
              <div className="shrink-0 flex items-center gap-3 px-3 h-10 border-b border-border bg-toolbar">
                <button
                  onClick={handleCompile}
                  disabled={compiling}
                  className="flex items-center gap-2 px-3 h-7 rounded bg-indigo-600 hover:bg-indigo-500 disabled:opacity-60 disabled:cursor-not-allowed text-white text-xs font-mono font-medium transition-colors"
                >
                  {compiling ? <Spinner /> : null}
                  {compiling ? "Compiling…" : "Compile"}
                </button>
                <span className="text-zinc-400 text-xs font-mono">⌘↵ / Ctrl+↵</span>
              </div>
              <div className="flex-1 min-h-0">
                <DslEditor ref={editorRef} onChange={handleEditorChange} onCompile={handleCompile} />
              </div>
            </Panel>
            
            <ResizeHandle direction="vertical" />
            
            <Panel
              panelRef={logsPanelRef}
              defaultSize={30}
              minSize={0}
              collapsible
              className="flex flex-col min-h-0"
            >
              <LogsPane log={log} onClear={() => setLog(null)} />
            </Panel>
          </Group>
        </Panel>

        <ResizeHandle direction="horizontal" />

        {/* Right Pane: Visualizer & Playback */}
        <Panel defaultSize={40} minSize={20} className="flex flex-col min-h-0 border-l border-border">
          <div className="h-10 shrink-0 border-b border-border">
            <PlaybackBar />
          </div>
          <div className="flex-1 min-h-0">
            <PianoRoll />
          </div>
        </Panel>
      </Group>
    </div>
  );
}
