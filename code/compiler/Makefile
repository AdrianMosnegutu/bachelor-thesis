BUILD_DIR        := build
EXAMPLE          := examples/example.dsl
UNIT_TESTS       := $(BUILD_DIR)/tests/unit_tests
INTEGRATION_TESTS := $(BUILD_DIR)/tests/integration/integration_tests

.PHONY: build run test test-unit test-integration compile-commands format clean

build:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(BUILD_DIR) -j

# Generate compile_commands.json and symlink it to the project root.
# Run this once after cloning; re-run whenever CMakeLists changes.
# Needed by clangd, CLion, VSCode (C/C++ extension), and other LSP clients.
compile-commands:
	cmake -B $(BUILD_DIR) -S . -DCMAKE_BUILD_TYPE=Debug
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

run: build
	./$(BUILD_DIR)/apps/dslrc $(EXAMPLE)

test-unit: build
	$(UNIT_TESTS)

test-integration: build
	$(INTEGRATION_TESTS)

test: build
	$(UNIT_TESTS)
	$(INTEGRATION_TESTS)

format:
	find . \( -path ./$(BUILD_DIR) -prune \) -o \( -name '*.hpp' -o -name '*.cpp' -o -name '*.h' -o -name '*.c' \) -print | xargs clang-format -i

clean:
	rm -rf $(BUILD_DIR)
