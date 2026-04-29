.PHONY: help up down build-compiler test-compiler dev-server dev-client install clean

COMPILER_DIR := compiler
BUILD_DIR    := $(COMPILER_DIR)/build
BUILD_TYPE   ?= Debug
NPROC        := $(shell nproc 2>/dev/null || echo 4)

help:
	@echo "Available targets:"
	@echo "  up               Build and start the full stack  →  https://localhost"
	@echo "  down             Stop and remove Docker Compose containers"
	@echo "  build-compiler   Configure and build dslrc (BUILD_TYPE=Debug|Release)"
	@echo "  test-compiler    Run the compiler test suite (CTest)"
	@echo "  dev-server       Start the Express backend in dev mode (tsx watch)"
	@echo "  dev-client       Start the Next.js frontend in dev mode"
	@echo "  install          Install npm dependencies for both server and client"
	@echo "  clean            Remove compiler build directory and node_modules dist"
	@echo ""
	@echo "  Browser will warn about the self-signed TLS cert on first visit."
	@echo "  Accept it once and the warning won't reappear."

# ── Docker ──────────────────────────────────────────────────────────────────

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    SUDO := sudo
else
    SUDO :=
endif

up:
	$(SUDO) docker compose up --build

down:
	$(SUDO) docker compose down

# ── Compiler ────────────────────────────────────────────────────────────────

build-compiler:
	cmake -S $(COMPILER_DIR) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	cmake --build $(BUILD_DIR) -j$(NPROC)

test-compiler: build-compiler
	cd $(BUILD_DIR) && ctest --output-on-failure -j$(NPROC)

# ── Dev servers ─────────────────────────────────────────────────────────────

dev-server:
	cd server && npm run dev

dev-client:
	cd client && npm run dev

# ── Dependencies ────────────────────────────────────────────────────────────

install:
	cd server && npm install
	cd client && npm install

# ── Cleanup ─────────────────────────────────────────────────────────────────

clean:
	rm -rf $(BUILD_DIR)
	rm -rf server/dist
	rm -rf client/.next
