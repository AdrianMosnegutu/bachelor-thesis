"use client";

import { useCallback, useEffect, useRef, useState } from "react";
import * as Tone from "tone";
import { useMidi } from "../context/MidiContext";

// ── Tuneable constants ──────────────────────────────────────────────────────────
// Edit GM_INSTRUMENT_MAP to change which soundfont instrument plays for each
// General MIDI program range. Instrument name strings must match the MIDI.js
// soundfont library (gleitz.github.io/midi-js-soundfonts).

type GmEntry = [minProgram: number, maxProgram: number, instrumentName: string];

const GM_INSTRUMENT_MAP: GmEntry[] = [
  [0,   7,   "acoustic_grand_piano"],
  [8,   15,  "celesta"],
  [16,  23,  "church_organ"],
  [24,  31,  "acoustic_guitar_nylon"],
  [32,  39,  "acoustic_bass"],
  [40,  47,  "violin"],
  [48,  55,  "string_ensemble_1"],
  [56,  63,  "trumpet"],
  [64,  71,  "alto_sax"],
  [72,  79,  "flute"],
];

const DEFAULT_INSTRUMENT = "acoustic_grand_piano";
const PERCUSSION_INSTRUMENT = "percussion";

function resolveInstrument(program: number, channel: number): string {
  if (channel === 9 || channel === 10) return PERCUSSION_INSTRUMENT;
  const entry = GM_INSTRUMENT_MAP.find(([min, max]) => program >= min && program <= max);
  return entry ? entry[2] : DEFAULT_INSTRUMENT;
}

// ── Types ───────────────────────────────────────────────────────────────────────

type PlayState = "stopped" | "playing" | "paused";
type LoadState = "idle" | "loading" | "ready" | "error";

// Minimal interface over soundfont-player's Player object
interface SfPlayer {
  play(note: string, when: number, options?: { duration?: number; gain?: number }): AudioBufferSourceNode;
  stop(when?: number): void;
}

// ── Icons ───────────────────────────────────────────────────────────────────────

function PauseIcon() {
  return (
    <svg width="14" height="14" viewBox="0 0 14 14" fill="currentColor">
      <rect x="2" y="1" width="4" height="12" rx="1" />
      <rect x="8" y="1" width="4" height="12" rx="1" />
    </svg>
  );
}
function PlayIcon() {
  return (
    <svg width="14" height="14" viewBox="0 0 14 14" fill="currentColor">
      <path d="M3 1.5l9 5.5-9 5.5V1.5z" />
    </svg>
  );
}
function StopIcon() {
  return (
    <svg width="14" height="14" viewBox="0 0 14 14" fill="currentColor">
      <rect x="2" y="2" width="10" height="10" rx="1" />
    </svg>
  );
}
function RewindIcon() {
  return (
    <svg width="14" height="14" viewBox="0 0 14 14" fill="currentColor">
      <path d="M2 2h2v10H2V2zm10 0L5 7l7 5V2z" />
    </svg>
  );
}

function Spinner() {
  return (
    <svg className="animate-spin w-3 h-3" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth={2.5}>
      <path strokeLinecap="round" d="M12 2a10 10 0 0 1 10 10" className="opacity-80" />
    </svg>
  );
}

// ── Component ───────────────────────────────────────────────────────────────────

export default function PlaybackBar() {
  const { midiBytes, parsedMidi } = useMidi();
  const [playState, setPlayState] = useState<PlayState>("stopped");
  const [loadState, setLoadState] = useState<LoadState>("idle");
  const [position, setPosition] = useState("0.0s / 0.0s");

  // Per-track soundfont players (index matches parsedMidi.tracks)
  const playersRef = useRef<SfPlayer[]>([]);
  // Audio nodes scheduled in the current play session (for pause/stop support)
  const scheduledNodesRef = useRef<AudioBufferSourceNode[]>([]);
  // Song position at the last pause, in seconds
  const pauseOffsetRef = useRef(0);
  const rafRef = useRef<number | null>(null);

  // ── stopAll ─────────────────────────────────────────────────────────────────

  const stopAll = useCallback(() => {
    scheduledNodesRef.current.forEach(node => {
      try { node.stop(0); } catch { /* already stopped — ignore */ }
    });
    scheduledNodesRef.current = [];

    playersRef.current.forEach(player => {
      try { player.stop(0); } catch { /* ignore */ }
    });

    Tone.getTransport().stop();
    Tone.getTransport().seconds = 0;
    pauseOffsetRef.current = 0;
    setPlayState("stopped");
    setPosition("0.0s");
  }, []);

  // ── Instrument loading ───────────────────────────────────────────────────────

  useEffect(() => {
    if (!parsedMidi) {
      setLoadState("idle");
      playersRef.current = [];
      return;
    }

    let cancelled = false;
    setLoadState("loading");
    stopAll();

    (async () => {
      try {
        const { default: Soundfont } = await import("soundfont-player");
        const ac = Tone.getContext().rawContext as AudioContext;

        const trackNames = parsedMidi.tracks.map(t =>
          resolveInstrument(t.instrument.number, t.channel)
        );
        const uniqueNames = [...new Set(trackNames)];

        const loaded = await Promise.all(
          uniqueNames.map(name => Soundfont.instrument(ac, name as Parameters<typeof Soundfont.instrument>[1]))
        );
        if (cancelled) return;

        const nameToPlayer = new Map(
          uniqueNames.map((name, i) => [name, loaded[i] as unknown as SfPlayer])
        );
        playersRef.current = trackNames.map(name => nameToPlayer.get(name)!);
        setLoadState("ready");
      } catch {
        if (!cancelled) setLoadState("error");
      }
    })();

    return () => { cancelled = true; };
  }, [parsedMidi, stopAll]);

  // ── Position display (RAF) ───────────────────────────────────────────────────

  useEffect(() => {
    const tick = () => {
      const current = Tone.getTransport().seconds;
      const total = parsedMidi?.duration ?? 0;
      setPosition(`${current.toFixed(1)}s / ${total.toFixed(1)}s`);

      if (playState === "playing" && parsedMidi && current >= parsedMidi.duration) {
        stopAll();
      }

      rafRef.current = requestAnimationFrame(tick);
    };
    rafRef.current = requestAnimationFrame(tick);
    return () => { if (rafRef.current) cancelAnimationFrame(rafRef.current); };
  }, [playState, parsedMidi, stopAll]);

  // ── Cleanup on unmount ───────────────────────────────────────────────────────

  useEffect(() => () => stopAll(), [stopAll]);

  // ── Note scheduling ──────────────────────────────────────────────────────────

  function scheduleNotes(ac: AudioContext, fromOffset: number) {
    if (!parsedMidi) return;
    scheduledNodesRef.current = [];
    // Small look-ahead buffer so the first note isn't missed
    const audioStart = ac.currentTime + 0.05;

    for (let ti = 0; ti < parsedMidi.tracks.length; ti++) {
      const player = playersRef.current[ti];
      if (!player) continue;

      for (const note of parsedMidi.tracks[ti].notes) {
        // Skip notes that already finished before the resume point
        if (note.time + note.duration < fromOffset) continue;

        const scheduleAt = audioStart + (note.time - fromOffset);
        if (scheduleAt < ac.currentTime) continue;

        const noteName = Tone.Frequency(note.midi, "midi").toNote();
        const node = player.play(noteName, scheduleAt, {
          duration: note.duration,
          gain: note.velocity,
        });
        scheduledNodesRef.current.push(node);
      }
    }
  }

  // ── Playback controls ────────────────────────────────────────────────────────

  async function handlePlay() {
    if (!parsedMidi || loadState !== "ready") return;

    await Tone.start();
    const ac = Tone.getContext().rawContext as AudioContext;
    const offset = pauseOffsetRef.current;

    if (playState === "paused") {
      scheduleNotes(ac, offset);
      Tone.getTransport().start();
      setPlayState("playing");
      return;
    }

    pauseOffsetRef.current = 0;
    Tone.getTransport().seconds = 0;
    scheduleNotes(ac, 0);
    Tone.getTransport().start();
    setPlayState("playing");
  }

  function handlePause() {
    const offset = Tone.getTransport().seconds;
    scheduledNodesRef.current.forEach(node => {
      try { node.stop(0); } catch { /* ignore */ }
    });
    scheduledNodesRef.current = [];
    playersRef.current.forEach(player => {
      try { player.stop(0); } catch { /* ignore */ }
    });
    Tone.getTransport().pause();
    pauseOffsetRef.current = offset;
    setPlayState("paused");
  }

  function handleRewind() {
    const isPlaying = playState === "playing";
    // Stop nodes first
    scheduledNodesRef.current.forEach(node => {
      try { node.stop(0); } catch { /* ignore */ }
    });
    scheduledNodesRef.current = [];
    playersRef.current.forEach(player => {
      try { player.stop(0); } catch { /* ignore */ }
    });

    Tone.getTransport().seconds = 0;
    pauseOffsetRef.current = 0;
    setPosition("0.0s");

    if (isPlaying) {
      handlePlay();
    } else {
      setPlayState("stopped");
    }
  }

  function handleSeek(e: React.ChangeEvent<HTMLInputElement>) {
    if (!parsedMidi) return;
    const seekTime = parseFloat(e.target.value);
    const isPlaying = playState === "playing";

    // Stop current scheduled nodes
    scheduledNodesRef.current.forEach(node => {
      try { node.stop(0); } catch { /* ignore */ }
    });
    scheduledNodesRef.current = [];

    Tone.getTransport().seconds = seekTime;
    pauseOffsetRef.current = seekTime;

    if (isPlaying) {
      const ac = Tone.getContext().rawContext as AudioContext;
      scheduleNotes(ac, seekTime);
    }
  }

  function handleDownload() {
    if (!midiBytes) return;
    const blob = new Blob([midiBytes.buffer as ArrayBuffer], { type: "audio/midi" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = "output.mid";
    a.click();
    URL.revokeObjectURL(url);
  }

  // ── Keyboard Shortcuts ──────────────────────────────────────────────────────

  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      // Don't toggle if user is typing in an input or the editor
      if (
        e.target instanceof HTMLInputElement ||
        e.target instanceof HTMLTextAreaElement ||
        (e.target as HTMLElement).isContentEditable ||
        (e.target as HTMLElement).classList.contains("monaco-editor") ||
        (e.target as HTMLElement).closest(".monaco-editor")
      ) {
        return;
      }

      if (e.code === "Space") {
        e.preventDefault();
        if (playState === "playing") {
          handlePause();
        } else {
          handlePlay();
        }
      }
    };

    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [playState, loadState, parsedMidi]); // Dependencies for handlePlay/handlePause logic

  // ── Render ───────────────────────────────────────────────────────────────────

  const btnBase = "flex items-center justify-center w-8 h-8 rounded transition-colors";
  const btnActive = "bg-zinc-200 dark:bg-zinc-700 hover:bg-zinc-300 dark:hover:bg-zinc-600 text-zinc-900 dark:text-zinc-100 cursor-pointer";
  const btnDisabled = "bg-zinc-100 dark:bg-zinc-800/50 text-zinc-400 dark:text-zinc-600 cursor-not-allowed";

  if (!parsedMidi) {
    return (
      <div className="flex items-center justify-center h-full text-zinc-500 font-mono text-xs">
        Compile successfully to enable playback
      </div>
    );
  }

  const canPlay = loadState === "ready";

  return (
    <div className="flex items-center gap-3 px-4 h-full bg-toolbar">
      {loadState === "loading" ? (
        <div className="flex items-center gap-2 text-zinc-500 font-mono text-xs">
          <Spinner />
          <span className="hidden sm:inline">Loading sounds…</span>
        </div>
      ) : loadState === "error" ? (
        <div className="text-red-600 dark:text-red-500 font-mono text-xs truncate max-w-[120px]" title="Failed to load instruments">
          Instrument Error
        </div>
      ) : (
        <div className="flex items-center gap-1">
          <button
            onClick={handleRewind}
            disabled={!canPlay}
            className={`${btnBase} ${canPlay ? btnActive : btnDisabled}`}
            title="Rewind to start"
          >
            <RewindIcon />
          </button>
          <button
            onClick={playState === "playing" ? handlePause : handlePlay}
            disabled={!canPlay}
            className={`${btnBase} ${canPlay ? btnActive : btnDisabled}`}
            title={playState === "playing" ? "Pause (Space)" : "Play (Space)"}
          >
            {playState === "playing" ? <PauseIcon /> : <PlayIcon />}
          </button>
          <button
            onClick={stopAll}
            disabled={playState === "stopped"}
            className={`${btnBase} ${playState === "stopped" ? btnDisabled : btnActive}`}
            title="Stop"
          >
            <StopIcon />
          </button>
        </div>
      )}

      {/* Progress Bar / Seeker */}
      <div className="flex-1 flex items-center gap-3 min-w-0">
        <input
          type="range"
          min="0"
          max={parsedMidi.duration}
          step="0.1"
          value={Tone.getTransport().seconds}
          onChange={handleSeek}
          className="flex-1 h-1 bg-zinc-300 dark:bg-zinc-800 rounded-lg appearance-none cursor-pointer accent-indigo-600"
          style={{
            background: `linear-gradient(to right, var(--color-indigo-600) ${(Tone.getTransport().seconds / parsedMidi.duration) * 100}%, var(--color-zinc-800) 0%)`
          }}
        />
        <span className="font-mono text-xs text-zinc-600 dark:text-zinc-200 whitespace-nowrap tabular-nums">
          {position}
        </span>
      </div>

      <button
        onClick={handleDownload}
        className="text-xs font-mono px-3 py-1.5 rounded bg-zinc-100 dark:bg-zinc-800 hover:bg-zinc-200 dark:hover:bg-zinc-700 text-zinc-700 dark:text-zinc-100 transition-colors border border-border shrink-0"
      >
        <span className="hidden sm:inline">Download .mid</span>
        <span className="sm:hidden">.mid</span>
      </button>
    </div>
  );
}
