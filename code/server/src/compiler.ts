import { execFile } from "child_process";
import { randomUUID } from "crypto";
import { readFile, unlink, writeFile } from "fs/promises";
import { tmpdir } from "os";
import { join } from "path";

export type CompileSuccess = { kind: "success"; midi: Buffer };
export type Diagnostic = {
  type: "lexical" | "syntax" | "semantic" | "lowering" | "output" | "internal";
  severity: "error" | "warning" | "note";
  message: string;
  location?: string;
  line?: number;
  column?: number;
};
export type CompileError = {
  kind: "error";
  diagnostics: Diagnostic[];
};
export type CompileResult = CompileSuccess | CompileError;

const ANSI_RE = /\x1B\[[0-9;]*m/g;

function parseStderr(raw: string): CompileError {
  const text = raw.replace(ANSI_RE, "").trim();
  const diagnostics: Diagnostic[] = [];

  for (const line of text.split("\n")) {
    const match = line.match(
      /:\s+(error|warning|note):\s+(lexical|syntax|semantic|lowering|output):\s+(?:(.+?):\s+)?(.+)$/i
    );

    if (match) {
      const [_, severity, type, locStr, message] = match;
      let lineNum: number | undefined;
      let colNum: number | undefined;

      if (locStr) {
        // Extract start line and column for editor highlighting
        const startMatch = locStr.match(/(\d+):(\d+)/);
        if (startMatch) {
          lineNum = parseInt(startMatch[1], 10);
          colNum = parseInt(startMatch[2], 10);
        }
      }

      diagnostics.push({
        severity: severity.toLowerCase() as "error" | "warning" | "note",
        type: type.toLowerCase() as
          | "lexical"
          | "syntax"
          | "semantic"
          | "lowering"
          | "output",
        message: message.trim(),
        location: locStr,
        line: lineNum,
        column: colNum,
      });
    } else if (line.trim()) {
      // Fallback for internal or unformatted errors
      diagnostics.push({
        severity: "error",
        type: "internal",
        message: line.trim(),
      });
    }
  }

  if (diagnostics.length === 0) {
    diagnostics.push({
      severity: "error",
      type: "internal",
      message: text || "unknown error",
    });
  }

  return { kind: "error", diagnostics };
}

export async function compile(source: string): Promise<CompileResult> {
  const bin = process.env.COMPILER_BIN ?? "/usr/local/bin/dslrc";
  const id = randomUUID();
  const src = join(tmpdir(), `dsl-${id}.dsl`);
  const out = join(tmpdir(), `dsl-${id}.mid`);
  const cleanup = () =>
    Promise.all([unlink(src).catch(() => {}), unlink(out).catch(() => {})]);

  await writeFile(src, source, "utf8");

  return new Promise<CompileResult>((resolve) => {
    execFile(bin, [src], async (err, _stdout, stderr) => {
      if (!err) {
        const midi = await readFile(out).catch(() => null);
        await cleanup();
        resolve(
          midi
            ? { kind: "success", midi }
            : {
                kind: "error",
                diagnostics: [
                  {
                    severity: "error",
                    type: "internal",
                    message: "output file not generated",
                  },
                ],
              }
        );
      } else if ((err as NodeJS.ErrnoException).code === "ENOENT") {
        await cleanup();
        resolve({
          kind: "error",
          diagnostics: [
            {
              severity: "error",
              type: "internal",
              message: "compiler binary not found",
            },
          ],
        });
      } else {
        await cleanup();
        resolve(parseStderr(stderr));
      }
    });
  });
}
