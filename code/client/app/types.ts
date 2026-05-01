export type Diagnostic = {
  type: "lexical" | "syntax" | "semantic" | "lowering" | "output" | "internal";
  severity: "error" | "warning" | "note";
  message: string;
  location?: string;
  line?: number;
  column?: number;
};

export type LogEntry =
  | { kind: "success"; timestamp: Date }
  | {
      kind: "error";
      diagnostics: Diagnostic[];
      timestamp: Date;
    };
