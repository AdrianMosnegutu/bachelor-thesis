import { execFile } from "child_process";
import { randomUUID } from "crypto";
import { readFile, unlink, writeFile } from "fs/promises";
import { tmpdir } from "os";
import { join } from "path";

export type CompileSuccess = { kind: "success"; midi: Buffer };
export type CompileError = {
  kind: "error";
  type: "lexical" | "syntax" | "semantic" | "internal";
  message: string;
  line?: number;
  column?: number;
};
export type CompileResult = CompileSuccess | CompileError;

const ANSI_RE = /\x1B\[[0-9;]*m/g;
const LOC_RE = /(\d+)\.(\d+)/;

function parseStderr(raw: string): CompileError {
  const text = raw.replace(ANSI_RE, "").trim();
  for (const line of text.split("\n")) {
    const m = line.match(/^(lexical|syntax|semantic) error:/i);
    if (!m) continue;
    const type = m[1].toLowerCase() as "lexical" | "syntax" | "semantic";
    const rest = line.slice(m[0].length).trim();
    const locMatch = rest.match(LOC_RE);
    const msgMatch = rest.match(/:\s+(.+)$/);
    return {
      kind: "error",
      type,
      message: msgMatch ? msgMatch[1].trim() : rest,
      ...(locMatch ? { line: +locMatch[1], column: +locMatch[2] } : {}),
    };
  }
  return { kind: "error", type: "internal", message: text || "unknown error" };
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
                type: "internal",
                message: "output file not generated",
              }
        );
      } else if ((err as NodeJS.ErrnoException).code === "ENOENT") {
        await cleanup();
        resolve({
          kind: "error",
          type: "internal",
          message: "compiler binary not found",
        });
      } else {
        await cleanup();
        resolve(parseStderr(stderr));
      }
    });
  });
}
