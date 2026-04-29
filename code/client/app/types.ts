export type LogEntry =
  | { kind: "success"; timestamp: Date }
  | {
      kind: "error";
      type: "lexical" | "syntax" | "semantic" | "internal";
      message: string;
      line?: number;
      column?: number;
      timestamp: Date;
    };
