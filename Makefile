CXX      = g++
CXXFLAGS = -std=c++17 -O3 -march=native -pthread -I.

SRC_DIR   = src
LIB_DIR   = lib
BUILD_DIR = build

LIB_SRCS := $(wildcard $(LIB_DIR)/*.cpp)
LIB_OBJS := $(patsubst $(LIB_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(LIB_SRCS))

MAIN_SRC := $(SRC_DIR)/main.cpp
MAIN_OBJ := $(BUILD_DIR)/main.o

TARGET     = $(BUILD_DIR)/main.out
INPUT_FILE = $(BUILD_DIR)/market_feed.bin

all: compile 

compile: lib main link

lib: $(LIB_OBJS)

$(BUILD_DIR)/%.o: $(LIB_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

main: $(MAIN_OBJ)

$(BUILD_DIR)/main.o: $(MAIN_SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

link: $(TARGET)

$(TARGET): $(MAIN_OBJ) $(LIB_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

run: $(TARGET) $(INPUT_FILE)
	./$(TARGET) < $(INPUT_FILE)

clean:
	rm -rf $(BUILD_DIR)/*

gentest: test_generator.py
	python3 test_generator.py

