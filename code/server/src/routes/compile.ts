import { Request, Response, Router } from "express";

import { compile } from "../compiler.js";

const router = Router();

router.post("/", async (req: Request, res: Response) => {
  const { source } = req.body as { source?: unknown };
  if (typeof source !== "string" || !source.trim()) {
    res
      .status(400)
      .json({ error: { type: "internal", message: "source is required" } });
    return;
  }

  const result = await compile(source);
  if (result.kind === "success") {
    res.set("Content-Type", "audio/midi");
    res.send(result.midi);
  } else {
    const status = result.diagnostics.some((d) => d.type === "internal")
      ? 500
      : 400;
    res.status(status).json({
      error: {
        kind: "error",
        diagnostics: result.diagnostics,
      },
    });
  }
});

export default router;
