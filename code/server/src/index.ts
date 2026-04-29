import cors from "cors";
import express from "express";

import compileRouter from "./routes/compile.js";
import healthRouter from "./routes/health.js";

const app = express();
const port = parseInt(process.env.PORT ?? "3001", 10);

app.use(cors());
app.use(express.json({ limit: "512kb" }));

app.use("/health", healthRouter);
app.use("/compile", compileRouter);

app.listen(port, () => {
  console.log(`Server listening on http://localhost:${port}`);
});
