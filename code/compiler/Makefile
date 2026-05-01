BUILD_DIR  := build
BUILD_TYPE ?= Debug
EXAMPLE    := examples/example.dsl
NPROC      := $(shell nproc 2>/dev/null || echo 4)

.PHONY: configure build run test test-unit test-integration compile-commands format clean

configure:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR) -j$(NPROC)

# Generate compile_commands.json and symlink it to the project root.
# Run this once after cloning; re-run whenever CMakeLists changes.
# Needed by clangd, CLion, VSCode (C/C++ extension), and other LSP clients.
compile-commands:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

run: build
	./$(BUILD_DIR)/apps/dslrc $(EXAMPLE)

test-unit: configure
	cmake --build $(BUILD_DIR) --target run_unit_tests -j$(NPROC)

test-integration: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure -j$(NPROC) -R "^(CompilerApi|CompilerApiDiagnostics|Integration|parse_example)"

test: configure
	cmake --build $(BUILD_DIR) --target run_integration_unit_tests -j$(NPROC)

format:
	find . \( -path ./$(BUILD_DIR) -prune \) -o \( -name '*.hpp' -o -name '*.cpp' -o -name '*.h' -o -name '*.c' \) -print | xargs clang-format -i

clean:
	rm -rf $(BUILD_DIR)
