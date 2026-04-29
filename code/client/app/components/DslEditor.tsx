"use client";

import Editor, { type BeforeMount, type OnMount, type Monaco } from "@monaco-editor/react";
import { forwardRef, useCallback, useImperativeHandle, useRef, useEffect } from "react";
import { useTheme } from "next-themes";

const STORAGE_KEY = "dsl-editor-content";

const DEFAULT_SNIPPET = `tempo 120;
signature 4/4;
key C major;

track {

}
`;

function registerDslLanguage(monaco: Monaco) {
  monaco.languages.register({ id: "dsl" });

  monaco.languages.setMonarchTokensProvider("dsl", {
    keywords: [
      "tempo",
      "signature",
      "key",
      "track",
      "pattern",
      "play",
      "for",
      "loop",
      "if",
      "else",
      "let",
      "using",
      "from",
      "rest",
      "major",
      "minor",
    ],
    instruments: ["piano", "guitar", "bass", "violin", "drums"],
    drumNotes: ["kick", "snare", "hihat", "crash", "ride"],
    booleans: ["true", "false"],

    tokenizer: {
      root: [
        // line comments
        [/\/\/.*$/, "comment"],
        // block comments
        [/\/\*/, "comment", "@blockComment"],
        // note literals: C4, A#3, Bb5, C#4, etc.
        [/\b[A-G][#b]?[0-9]\b/, "note-literal"],
        // strings
        [/"([^"\\]|\\.)*"/, "string"],
        // numbers
        [/\b\d+(\.\d+)?\b/, "number"],
        // identifiers / keywords
        [
          /\b[a-z_][a-zA-Z0-9_]*\b/,
          {
            cases: {
              "@keywords": "keyword",
              "@instruments": "instrument",
              "@drumNotes": "drum-note",
              "@booleans": "boolean-literal",
              "@default": "identifier",
            },
          },
        ],
        // operators
        [/[+\-*/%<>!=&|]+/, "operator"],
        // punctuation
        [/[{}();,]/, "delimiter"],
      ],
      blockComment: [
        [/[^/*]+/, "comment"],
        [/\*\//, "comment", "@pop"],
        [/[/*]/, "comment"],
      ],
    },
  });

  monaco.editor.defineTheme("dsl-dark", {
    base: "vs-dark",
    inherit: true,
    rules: [
      { token: "keyword", foreground: "c792ea", fontStyle: "bold" },
      { token: "note-literal", foreground: "f78c6c" },
      { token: "instrument", foreground: "89ddff" },
      { token: "drum-note", foreground: "89ddff" },
      { token: "boolean-literal", foreground: "ff5370" },
      { token: "string", foreground: "c3e88d" },
      { token: "number", foreground: "f78c6c" },
      { token: "comment", foreground: "94a2a8", fontStyle: "italic" },
      { token: "operator", foreground: "89ddff" },
      { token: "identifier", foreground: "eeffff" },
    ],
    colors: {
      "editor.background": "#0d0d0f",
      "editor.foreground": "#eeffff",
      "editorLineNumber.foreground": "#8a8a8a",
      "editorLineNumber.activeForeground": "#ffffff",
      "editor.selectionBackground": "#1a2a3a",
      "editor.lineHighlightBackground": "#12121a",
    },
  });

  monaco.editor.defineTheme("dsl-light", {
    base: "vs",
    inherit: true,
    rules: [
      { token: "keyword", foreground: "7c3aed", fontStyle: "bold" }, // violet
      { token: "note-literal", foreground: "ea580c" }, // orange
      { token: "instrument", foreground: "0891b2" }, // cyan
      { token: "drum-note", foreground: "0891b2" },
      { token: "boolean-literal", foreground: "dc2626" }, // red
      { token: "string", foreground: "16a34a" }, // green
      { token: "number", foreground: "ea580c" },
      { token: "comment", foreground: "52525b", fontStyle: "italic" }, // zinc-600
      { token: "operator", foreground: "0891b2" },
      { token: "identifier", foreground: "#09090b" },
    ],
    colors: {
      "editor.background": "#ffffff",
      "editor.foreground": "#09090b",
      "editorLineNumber.foreground": "#71717a", // zinc-500
      "editorLineNumber.activeForeground": "#18181b", // zinc-900
      "editor.selectionBackground": "#e4e4e7",
      "editor.lineHighlightBackground": "#f4f4f5",
    },
  });
}

export interface DslEditorHandle {
  jumpTo: (line: number, column: number) => void;
  blur: () => void;
  setError: (line: number, column: number, message: string) => void;
  clearError: () => void;
}

interface DslEditorProps {
  onChange?: (value: string) => void;
  onCompile?: () => void;
}

const DslEditor = forwardRef<DslEditorHandle, DslEditorProps>(function DslEditor(
  { onChange, onCompile },
  ref
) {
  const { theme } = useTheme();
  const debounceRef = useRef<ReturnType<typeof setTimeout> | null>(null);
  const onCompileRef = useRef(onCompile);
  onCompileRef.current = onCompile;

  const editorInstanceRef = useRef<Parameters<OnMount>[0] | null>(null);
  const monacoRef = useRef<Monaco | null>(null);

  useEffect(() => {
    if (editorInstanceRef.current && monacoRef.current) {
      monacoRef.current.editor.setTheme(theme === "dark" ? "dsl-dark" : "dsl-light");
    }
  }, [theme]);

  useImperativeHandle(ref, () => ({
    jumpTo(line: number, column: number) {
      const editor = editorInstanceRef.current;
      if (!editor) return;
      const pos = { lineNumber: line, column: column };
      editor.setPosition(pos);
      editor.revealPositionInCenter(pos);
      editor.focus();
    },
    blur() {
      const editor = editorInstanceRef.current;
      if (!editor) return;
      if (document.activeElement instanceof HTMLElement) {
        document.activeElement.blur();
      }
    },
    setError(line: number, column: number, message: string) {
      const editor = editorInstanceRef.current;
      const monaco = monacoRef.current;
      if (!editor || !monaco) return;

      const model = editor.getModel();
      if (!model) return;

      monaco.editor.setModelMarkers(model, "dsl", [
        {
          startLineNumber: line,
          startColumn: column,
          endLineNumber: line,
          endColumn: column + 1,
          message: message,
          severity: monaco.MarkerSeverity.Error,
        },
      ]);
    },
    clearError() {
      const editor = editorInstanceRef.current;
      const monaco = monacoRef.current;
      if (!editor || !monaco) return;

      const model = editor.getModel();
      if (!model) return;

      monaco.editor.setModelMarkers(model, "dsl", []);
    },
  }));

  const handleBeforeMount: BeforeMount = useCallback((m) => {
    registerDslLanguage(m);
  }, []);

  const initialValue =
    (typeof window !== "undefined" && localStorage.getItem(STORAGE_KEY)) ||
    DEFAULT_SNIPPET;

  const handleChange = useCallback(
    (value: string | undefined) => {
      const v = value ?? "";
      if (debounceRef.current) clearTimeout(debounceRef.current);
      debounceRef.current = setTimeout(() => {
        localStorage.setItem(STORAGE_KEY, v);
      }, 300);
      onChange?.(v);
    },
    [onChange]
  );

  const onChangeRef = useRef(onChange);
  onChangeRef.current = onChange;

  const handleMount: OnMount = useCallback(
    (editor, m) => {
      editorInstanceRef.current = editor;
      monacoRef.current = m;
      onChangeRef.current?.(editor.getValue());

      editor.addAction({
        id: "dsl.compile",
        label: "Compile",
        keybindings: [m.KeyMod.CtrlCmd | m.KeyCode.Enter],
        run: () => onCompileRef.current?.(),
      });
    },
    []
  );

  return (
    <Editor
      height="100%"
      defaultLanguage="dsl"
      defaultValue={initialValue}
      theme={theme === "dark" ? "dsl-dark" : "dsl-light"}
      beforeMount={handleBeforeMount}
      onChange={handleChange}
      onMount={handleMount}
      options={{
        fontSize: 14,
        fontFamily: "var(--font-geist-mono), ui-monospace, monospace",
        minimap: { enabled: false },
        scrollBeyondLastLine: false,
        lineNumbers: "on",
        renderLineHighlight: "line",
        padding: { top: 16, bottom: 16 },
        overviewRulerLanes: 0,
      }}
    />
  );
});

export default DslEditor;
