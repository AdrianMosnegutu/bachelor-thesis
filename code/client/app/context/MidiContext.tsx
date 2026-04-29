"use client";

import React, { createContext, useContext, useMemo, useState } from "react";
import { Midi } from "@tonejs/midi";

type MidiContextValue = {
  midiBytes: Uint8Array | null;
  setMidiBytes: (bytes: Uint8Array | null) => void;
  parsedMidi: Midi | null;
};

const MidiContext = createContext<MidiContextValue | null>(null);

export function MidiProvider({ children }: { children: React.ReactNode }) {
  const [midiBytes, setMidiBytes] = useState<Uint8Array | null>(null);

  const parsedMidi = useMemo(() => {
    if (!midiBytes) return null;
    try {
      return new Midi(
        midiBytes.buffer.slice(
          midiBytes.byteOffset,
          midiBytes.byteOffset + midiBytes.byteLength
        ) as ArrayBuffer
      );
    } catch {
      return null;
    }
  }, [midiBytes]);

  return (
    <MidiContext.Provider value={{ midiBytes, setMidiBytes, parsedMidi }}>
      {children}
    </MidiContext.Provider>
  );
}

export function useMidi() {
  const ctx = useContext(MidiContext);
  if (!ctx) throw new Error("useMidi must be used within MidiProvider");
  return ctx;
}
