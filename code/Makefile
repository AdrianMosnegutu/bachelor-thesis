.PHONY: help up down build-compiler test-compiler coverage-compiler coverage-clean-compiler dev-server dev-client install clean

COMPILER_DIR := compiler
COVERAGE     ?= OFF
BUILD_DIR    ?= $(if $(filter ON on 1 true yes,$(COVERAGE)),$(COMPILER_DIR)/build-coverage,$(COMPILER_DIR)/build)
BUILD_TYPE   ?= Debug
NPROC        := $(shell nproc 2>/dev/null || echo 4)
COVERAGE_ON  = $(if $(filter ON on 1 true yes,$(COVERAGE)),ON,OFF)
COVERAGE_RAW = $(BUILD_DIR)/coverage.info
COVERAGE_FILTERED = $(BUILD_DIR)/coverage.filtered.info
COVERAGE_HTML = $(BUILD_DIR)/coverage-html
COVERAGE_BUILD_DIR = $(abspath $(BUILD_DIR))

define reset_coverage_counters
	@if [ "$(COVERAGE_ON)" = "ON" ]; then \
		lcov --zerocounters --directory "$(BUILD_DIR)"; \
	fi
endef

define maybe_generate_coverage_report
	@if [ "$(COVERAGE_ON)" = "ON" ]; then \
		set -e; \
		lcov --capture --ignore-errors inconsistent --directory "$(BUILD_DIR)" --output-file "$(COVERAGE_RAW)"; \
		lcov --remove "$(COVERAGE_RAW)" "/usr/*" "$(COVERAGE_BUILD_DIR)/*" "*/tests/*" --output-file "$(COVERAGE_FILTERED)"; \
		genhtml --ignore-errors range "$(COVERAGE_FILTERED)" --output-directory "$(COVERAGE_HTML)"; \
		echo "Coverage report: $(COVERAGE_HTML)/index.html"; \
	fi
endef

define run_compiler_tests_and_maybe_generate_coverage_report
	@status=0; \
	cd "$(BUILD_DIR)" && ctest --output-on-failure -j$(NPROC) || status=$$?; \
	if [ "$(COVERAGE_ON)" = "ON" ]; then \
		set -e; \
		lcov --capture --ignore-errors inconsistent --directory "$(BUILD_DIR)" --output-file "$(COVERAGE_RAW)"; \
		lcov --remove "$(COVERAGE_RAW)" "/usr/*" "$(COVERAGE_BUILD_DIR)/*" "*/tests/*" --output-file "$(COVERAGE_FILTERED)"; \
		genhtml --ignore-errors range "$(COVERAGE_FILTERED)" --output-directory "$(COVERAGE_HTML)"; \
		echo "Coverage report: $(COVERAGE_HTML)/index.html"; \
	fi; \
	exit $$status
endef

help:
	@echo "Available targets:"
	@echo "  up               Build and start the full stack  →  https://localhost"
	@echo "  down             Stop and remove Docker Compose containers"
	@echo "  build-compiler   Configure and build dslrc (BUILD_TYPE=Debug|Release COVERAGE=ON|OFF)"
	@echo "  test-compiler    Run compiler tests (COVERAGE=ON also writes an lcov HTML report)"
	@echo "  coverage-compiler Run compiler tests with coverage reporting enabled"
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
	cmake -S $(COMPILER_DIR) -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DDSL_ENABLE_COVERAGE=$(COVERAGE_ON)
	cmake --build $(BUILD_DIR) -j$(NPROC)

test-compiler: build-compiler
	$(reset_coverage_counters)
	$(run_compiler_tests_and_maybe_generate_coverage_report)

coverage-compiler: COVERAGE := ON
coverage-compiler: test-compiler

coverage-clean-compiler:
	rm -rf $(COMPILER_DIR)/build-coverage

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
