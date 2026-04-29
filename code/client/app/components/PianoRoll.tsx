"use client";

import { useEffect, useMemo, useRef, useState } from "react";
import * as Tone from "tone";
import { useTheme } from "next-themes";
import { useMidi } from "../context/MidiContext";

// ─── Tuneable constants ────────────────────────────────────────────────────────

const TRACK_COLORS = [
  "#6366f1", // indigo
  "#22d3ee", // cyan
  "#f59e0b", // amber
  "#10b981", // emerald
  "#f43f5e", // rose
  "#a855f7", // purple
  "#84cc16", // lime
  "#f97316", // orange
];

const PIXELS_PER_SEC = 150;
const NOTE_GAP = 1;
const KEY_STRIP_HEIGHT = 80;

// ─── Types ────────────────────────────────────────────────────────────────────

type MidiTrack = NonNullable<ReturnType<typeof useMidi>["parsedMidi"]>["tracks"][number];

// ─── Hooks ────────────────────────────────────────────────────────────────────

function useCanvasSize(ref: React.RefObject<HTMLDivElement | null>) {
  const [size, setSize] = useState({ width: 0, height: 0 });
  useEffect(() => {
    const el = ref.current;
    if (!el) return;
    const update = () => setSize({ width: el.offsetWidth, height: el.offsetHeight });
    update();
    const ro = new ResizeObserver(update);
    ro.observe(el);
    return () => ro.disconnect();
  }, [ref]);
  return size;
}

function usePlayheadTime() {
  const [time, setTime] = useState(0);
  useEffect(() => {
    let rafId: number;
    const tick = () => {
      setTime(Tone.getTransport().seconds);
      rafId = requestAnimationFrame(tick);
    };
    rafId = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(rafId);
  }, []);
  return time;
}

function useNoteRange(tracks: MidiTrack[] | null) {
  return useMemo(() => {
    if (!tracks || tracks.length === 0) return { minMidi: 48, maxMidi: 84 };
    let min = 127;
    let max = 0;
    for (const track of tracks) {
      for (const note of track.notes) {
        if (note.midi < min) min = note.midi;
        if (note.midi > max) max = note.midi;
      }
    }
    return { minMidi: Math.max(0, min - 2), maxMidi: Math.min(127, max + 2) };
  }, [tracks]);
}

// ─── Piano key helpers ────────────────────────────────────────────────────────

const BLACK_KEY_OFFSETS = new Set([1, 3, 6, 8, 10]); // semitone positions within octave

function isBlackKey(midi: number) {
  return BLACK_KEY_OFFSETS.has(midi % 12);
}

// ─── Renderer ─────────────────────────────────────────────────────────────────

function renderFrame(
  ctx: CanvasRenderingContext2D,
  tracks: MidiTrack[],
  currentTime: number,
  width: number,
  height: number,
  minMidi: number,
  maxMidi: number,
  theme: string | undefined
) {
  const isDark = theme === "dark";
  const noteRange = maxMidi - minMidi + 1;
  const colWidth = width / noteRange;
  const keyStripY = height - KEY_STRIP_HEIGHT;
  const nowLineY = keyStripY;

  // Background
  ctx.fillStyle = isDark ? "#0d0d0f" : "#ffffff";
  ctx.fillRect(0, 0, width, height);

  // Subtle grid lines for octave boundaries
  ctx.strokeStyle = isDark ? "#1a1a2e" : "#f0f0f0";
  ctx.lineWidth = 1;
  for (let midi = minMidi; midi <= maxMidi; midi++) {
    if (midi % 12 === 0) {
      const x = (midi - minMidi) * colWidth;
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, keyStripY);
      ctx.stroke();
    }
  }

  // Build active-note set for key strip highlighting
  const activeNotes = new Map<number, string>(); // midi → colour

  // Note blocks
  for (let ti = 0; ti < tracks.length; ti++) {
    const color = TRACK_COLORS[ti % TRACK_COLORS.length];
    const track = tracks[ti];

    for (const note of track.notes) {
      const noteTop = nowLineY - (note.time - currentTime) * PIXELS_PER_SEC;
      const noteBottom = nowLineY - (note.time + note.duration - currentTime) * PIXELS_PER_SEC;

      // Skip notes outside the visible area
      if (noteBottom > keyStripY || noteTop < 0) continue;

      const noteX = (note.midi - minMidi) * colWidth;
      const noteWidth = Math.max(colWidth - NOTE_GAP, 2);

      // Clip to key strip top
      const clippedTop = Math.max(noteBottom, 0);
      const clippedHeight = Math.min(noteTop, keyStripY) - clippedTop;
      if (clippedHeight <= 0) continue;

      ctx.fillStyle = color;
      ctx.fillRect(noteX, clippedTop, noteWidth, clippedHeight);

      // Round cap at note head (the part nearest the now line)
      const capRadius = Math.min(noteWidth / 2, 3);
      const capY = Math.min(noteTop - capRadius, keyStripY - capRadius);
      if (capY >= clippedTop) {
        ctx.beginPath();
        ctx.arc(noteX + noteWidth / 2, capY, capRadius, 0, Math.PI * 2);
        ctx.fill();
      }

      // Track which notes are currently sounding for key strip
      if (note.time <= currentTime && note.time + note.duration >= currentTime) {
        activeNotes.set(note.midi, color);
      }
    }
  }

  // Now line
  ctx.strokeStyle = isDark ? "rgba(255,255,255,0.35)" : "rgba(0,0,0,0.15)";
  ctx.lineWidth = 1.5;
  ctx.beginPath();
  ctx.moveTo(0, nowLineY);
  ctx.lineTo(width, nowLineY);
  ctx.stroke();

  // Piano key strip background
  ctx.fillStyle = isDark ? "#111114" : "#f8f8f8";
  ctx.fillRect(0, keyStripY, width, KEY_STRIP_HEIGHT);
  ctx.strokeStyle = isDark ? "#2a2a3a" : "#e4e4e7";
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(0, keyStripY);
  ctx.lineTo(width, keyStripY);
  ctx.stroke();

  // Draw keys in a "wedged" layout like a real piano
  const whiteKeyColor = isDark ? "#e8e8e8" : "#ffffff";
  const whiteKeyBorder = isDark ? "#333344" : "#d1d1d6";

  // 1. Draw the base white keys (filling the strip first)
  ctx.fillStyle = whiteKeyColor;
  ctx.fillRect(0, keyStripY + 1, width, KEY_STRIP_HEIGHT - 2);

  // 2. Draw active highlights for white keys
  for (let midi = minMidi; midi <= maxMidi; midi++) {
    if (isBlackKey(midi)) continue;
    const activeColor = activeNotes.get(midi);
    if (!activeColor) continue;

    let xStart = (midi - minMidi) * colWidth;
    let xEnd = xStart + colWidth;

    // Expand to the middle of adjacent black keys
    if (midi > 0 && isBlackKey(midi - 1)) xStart -= colWidth / 2;
    if (midi < 127 && isBlackKey(midi + 1)) xEnd += colWidth / 2;

    const x = Math.max(0, xStart);
    const w = Math.min(width, xEnd) - x;

    ctx.fillStyle = activeColor;
    ctx.fillRect(x + 0.5, keyStripY + 1, w - 1, KEY_STRIP_HEIGHT - 2);
  }

  // 3. Draw separator lines between white keys
  ctx.strokeStyle = whiteKeyBorder;
  ctx.lineWidth = 0.5;
  for (let midi = minMidi; midi <= maxMidi; midi++) {
    const isNextBlack = midi < 127 && isBlackKey(midi + 1);
    const isThisBlack = isBlackKey(midi);
    
    let lineX: number | null = null;
    if (!isThisBlack && !isNextBlack && midi < maxMidi) {
      // Line between two white keys (E-F or B-C)
      lineX = (midi + 1 - minMidi) * colWidth;
    } else if (isThisBlack) {
      // Line at the center of a black key
      lineX = (midi - minMidi) * colWidth + colWidth / 2;
    }

    if (lineX !== null && lineX > 0 && lineX < width) {
      ctx.beginPath();
      ctx.moveTo(lineX, keyStripY + 1);
      ctx.lineTo(lineX, keyStripY + KEY_STRIP_HEIGHT - 1);
      ctx.stroke();
    }
  }

  // 4. Draw black keys on top
  for (let midi = minMidi; midi <= maxMidi; midi++) {
    if (!isBlackKey(midi)) continue;
    const x = (midi - minMidi) * colWidth;
    const activeColor = activeNotes.get(midi);
    const blackKeyColor = activeColor ? activeColor : (isDark ? "#1a1a1a" : "#2c2c2e");
    
    const bw = Math.max(colWidth * 0.7, 1);
    const bx = x + (colWidth - bw) / 2;
    ctx.fillStyle = blackKeyColor;
    ctx.fillRect(bx, keyStripY + 1, bw, KEY_STRIP_HEIGHT * 0.6);
    
    // Add a small highlight/border to black keys in dark mode for better definition
    if (isDark && !activeColor) {
      ctx.strokeStyle = "#333344";
      ctx.lineWidth = 0.5;
      ctx.strokeRect(bx, keyStripY + 1, bw, KEY_STRIP_HEIGHT * 0.6);
    }
  }

  // 5. Annotate white keys with pitches
  ctx.font = "bold 9px ui-monospace, monospace";
  ctx.textAlign = "center";
  
  for (let midi = minMidi; midi <= maxMidi; midi++) {
    if (isBlackKey(midi)) continue;
    const activeColor = activeNotes.get(midi);
    
    const noteNames = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
    const octave = Math.floor(midi / 12) - 1;
    const label = `${noteNames[midi % 12]}${octave}`;
    
    if (colWidth > 12) {
      // Calculate the actual horizontal center of the "wedged" white key
      let xStart = (midi - minMidi) * colWidth;
      let xEnd = xStart + colWidth;
      if (midi > 0 && isBlackKey(midi - 1)) xStart -= colWidth / 2;
      if (midi < 127 && isBlackKey(midi + 1)) xEnd += colWidth / 2;
      
      const centerX = (xStart + xEnd) / 2;
      
      ctx.fillStyle = activeColor ? "rgba(255,255,255,0.9)" : (isDark ? "rgba(0,0,0,0.45)" : "rgba(0,0,0,0.3)");
      // Position in the bottom 40% of the key (below the black keys)
      const bottomPartTop = keyStripY + KEY_STRIP_HEIGHT * 0.6;
      ctx.fillText(label, centerX, bottomPartTop + (KEY_STRIP_HEIGHT * 0.4) / 2 + 4);
    }
  }
}

// ─── Component ────────────────────────────────────────────────────────────────

export default function PianoRoll() {
  const { resolvedTheme } = useTheme();
  const isDark = resolvedTheme === "dark";
  const { parsedMidi } = useMidi();
  const containerRef = useRef<HTMLDivElement>(null);
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const { width, height } = useCanvasSize(containerRef);
  const currentTime = usePlayheadTime();
  const { minMidi, maxMidi } = useNoteRange(parsedMidi?.tracks ?? null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || !parsedMidi || width === 0 || height === 0) return;
    const ctx = canvas.getContext("2d");
    if (!ctx) return;
    renderFrame(ctx, parsedMidi.tracks, currentTime, width, height, minMidi, maxMidi, resolvedTheme);
  }, [parsedMidi, currentTime, width, height, minMidi, maxMidi, resolvedTheme]);

  return (
    <div ref={containerRef} className="w-full h-full bg-panel overflow-hidden">
      {!parsedMidi ? (
        <div className="flex items-center justify-center h-full text-zinc-400 font-mono text-xs text-center px-4">
          Compile your code to see the visualizer
        </div>
      ) : (
        width > 0 && height > 0 && (
          <canvas
            ref={canvasRef}
            width={width}
            height={height}
            style={{ display: "block" }}
          />
        )
      )}
    </div>
  );
}
