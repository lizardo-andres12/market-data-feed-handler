# Wrapper Makefile for CMake
# This allows you to run "make run" or "make test" from the root
# without worrying about cmake commands.

BUILD_DIR := build
EXECUTABLE := $(BUILD_DIR)/main.out
INPUT_FILE := $(BUILD_DIR)/market_feed.bin
TEST_GEN_SCRIPT := test_generator.py

TYPE ?= Release
EXTRA_FLAGS ?=
TSAN ?= OFF

.PHONY: all build run test clean

all: build

build:
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(TYPE) \
		-DCMAKE_CXX_FLAGS="$(EXTRA_FLAGS)" \
		-DENABLE_TSAN=$(TSAN)
	@cmake --build $(BUILD_DIR)
	@cp $(BUILD_DIR)/compile_commands.json .  # So clangd LSP stops complaining about `#include` paths

run: $(EXECUTABLE)
	./$(EXECUTABLE) < $(INPUT_FILE)

tgen: $(TEST_GEN_SCRIPT)
	python3 $(TEST_GEN_SCRIPT)

test: $(BUILD_DIR)
	@cd $(BUILD_DIR) && ctest --output-on-failure

test-tsan:
	@echo "Building with TSan enabled..."
	@$(MAKE) clean
	@$(MAKE) build TSAN=ON TYPE=Debug
	@echo "Running tests with TSan..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	@rm -rf $(BUILD_DIR)/*

