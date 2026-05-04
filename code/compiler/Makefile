BUILD_DIR  ?= $(if $(filter ON on 1 true yes,$(COVERAGE)),build-coverage,build)
BUILD_TYPE ?= Debug
COVERAGE   ?= OFF
EXAMPLE    := examples/example.dsl
NPROC      := $(shell nproc 2>/dev/null || echo 4)

CTEST      = ctest --test-dir $(BUILD_DIR) --output-on-failure -j$(NPROC)
COVERAGE_ON = $(if $(filter ON on 1 true yes,$(COVERAGE)),ON,OFF)
CMAKE_COVERAGE_FLAG = -DDSL_ENABLE_COVERAGE=$(COVERAGE_ON)
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

define run_tests_and_maybe_generate_coverage_report
	@status=0; \
	$(CTEST) $(1) || status=$$?; \
	if [ "$(COVERAGE_ON)" = "ON" ]; then \
		set -e; \
		lcov --capture --ignore-errors inconsistent --directory "$(BUILD_DIR)" --output-file "$(COVERAGE_RAW)"; \
		lcov --remove "$(COVERAGE_RAW)" "/usr/*" "$(COVERAGE_BUILD_DIR)/*" "*/tests/*" --output-file "$(COVERAGE_FILTERED)"; \
		genhtml --ignore-errors range "$(COVERAGE_FILTERED)" --output-directory "$(COVERAGE_HTML)"; \
		echo "Coverage report: $(COVERAGE_HTML)/index.html"; \
	fi; \
	exit $$status
endef

COMMON_SUITES 	 := Note
PARSING_SUITES   := Parser|ParserDiagnostics
SEMANTIC_SUITES  := ArithmeticTypeCheck|ChordTypeCheck|ComparisonTypeCheck|ControlFlow|LetDeclaration|LogicalTypeCheck|Metadata|PatternDeclaration|PatternScoping|PlayStatement|ScopeAccess|SequenceTypeCheck|Shadowing|TernaryTypeCheck|TrackDeclaration|UnaryTypeCheck|VariableScoping
LOWERING_SUITES  := ChordCursor|ForLoop|FromOffset|IfStatement|LoopStatement|LowererDiagnostics|LoweringRegression|NoteCursor|PatternCall|RestCursor|RuntimeErrors|SequenceCursor|VoiceParallel
MIDI_SUITES      := MidiWriter
INTEGRATION_SUITES := CompilerApi|CompilerApiDiagnostics|Integration|parse_example

.PHONY: configure build run \
        test test-unit test-integration \
        test-parsing test-semantic test-lowering test-midi \
        coverage coverage-clean compile-commands format clean

configure:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_COVERAGE_FLAG)

build: configure
	cmake --build $(BUILD_DIR) -j$(NPROC)

# Generate compile_commands.json and symlink it to the project root.
# Run this once after cloning; re-run whenever CMakeLists changes.
# Needed by clangd, CLion, VSCode (C/C++ extension), and other LSP clients.
compile-commands:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_COVERAGE_FLAG)
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

run: build
	./$(BUILD_DIR)/apps/dslrc $(EXAMPLE)

# -- Per-layer test targets ----------------------------------------------------

test-common: configure
	cmake --build $(BUILD_DIR) --target common_tests -j$(NPROC)
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report,-R "^($(COMMON_SUITES))\.")

test-parsing: configure
	cmake --build $(BUILD_DIR) --target parsing_tests -j$(NPROC)
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report,-R "^($(PARSING_SUITES))\.")

test-semantic: configure
	cmake --build $(BUILD_DIR) --target semantic_tests -j$(NPROC)
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report,-R "^($(SEMANTIC_SUITES))\.")

test-lowering: configure
	cmake --build $(BUILD_DIR) --target ir_tests -j$(NPROC)
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report,-R "^($(LOWERING_SUITES))\.")

test-midi: configure
	cmake --build $(BUILD_DIR) --target midi_tests -j$(NPROC)
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report,-R "^($(MIDI_SUITES))\.")

# -- Aggregate test targets ----------------------------------------------------

test-unit: build
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report,-E "^($(INTEGRATION_SUITES))\.")

test-integration: build
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report,-R "^($(INTEGRATION_SUITES))\.")

test: build
	$(reset_coverage_counters)
	$(call run_tests_and_maybe_generate_coverage_report)

coverage: COVERAGE := ON
coverage: test

coverage-clean:
	rm -rf build-coverage

format:
	find . \( -path ./$(BUILD_DIR) -prune \) -o \( -name '*.hpp' -o -name '*.cpp' -o -name '*.h' -o -name '*.c' \) -print | xargs clang-format -i

clean:
	rm -rf $(BUILD_DIR)
