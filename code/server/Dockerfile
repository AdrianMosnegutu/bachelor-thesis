# ── Stage 1: build the C++ compiler ──────────────────────────────────────────
FROM debian:bookworm-slim AS compiler-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake flex bison g++ make ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /compiler
COPY compiler/ .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF \
    && cmake --build build -j$(nproc)

# ── Stage 2: compile TypeScript ───────────────────────────────────────────────
FROM node:20-slim AS ts-builder

WORKDIR /app
COPY server/package*.json ./
RUN npm ci

COPY server/ .
RUN npm run build

# ── Stage 3: lean runtime ─────────────────────────────────────────────────────
FROM node:20-slim AS runtime

WORKDIR /app

COPY server/package*.json ./
RUN npm ci --omit=dev

COPY --from=ts-builder /app/dist ./dist
COPY --from=compiler-builder /compiler/build/apps/dslrc /usr/local/bin/dslrc

ENV NODE_ENV=production
ENV PORT=3001
ENV COMPILER_BIN=/usr/local/bin/dslrc

EXPOSE 3001
CMD ["node", "dist/index.js"]
