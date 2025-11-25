CXX      := g++
CXXFLAGS := -std=c++17 -O3 -march=native -pthread -Wall -Wextra -Iinclude -MMD -MP

SRC_DIR   := src
BUILD_DIR := build
BIN_DIR   := $(BUILD_DIR)/bin

# Recursive search for source files in src/ and its subdirectories
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

# Target executable
TARGET     := $(BIN_DIR)/main.out
INPUT_FILE := $(BUILD_DIR)/market_feed.bin 

.PHONY: all clean run gentest directories

all: directories $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

directories:
	@mkdir -p $(BIN_DIR)

run: $(TARGET)
	./$(TARGET) < $(INPUT_FILE)

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

gentest: test_generator.py
	python3 test_generator.py

-include $(DEPS)
