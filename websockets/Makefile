# Compiler and Flags
CXX = clang++
CXXFLAGS = -std=c++20 -I../include -I/opt/homebrew/include -I/opt/homebrew/opt/openssl/include -pthread

# Directories
SRC_DIR = ../src
INCLUDE_DIR = ../include
OBJ_DIR = ./obj
BIN_DIR = ./bin
WS_SRC_DIR = ./src
WS_INCLUDE_DIR = ./include

# Libraries
LDFLAGS = -L/opt/homebrew/lib -L/opt/homebrew/opt/openssl/lib
LDLIBS = -lssl -lcrypto -lboost_system -lboost_json

# Sources and Objects
SRC_FILES = $(SRC_DIR)/simdjson.cpp $(WS_SRC_DIR)/orderbook.cpp
TEST_FILE = ./test.cpp
OBJS = $(OBJ_DIR)/simdjson.o $(OBJ_DIR)/orderbook.o $(OBJ_DIR)/test.o

# Headers
HEADERS = $(WS_INCLUDE_DIR)/orderbook.hpp $(INCLUDE_DIR)/data.hpp

# Target
TARGET = $(BIN_DIR)/test

# Build Rules
all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(OBJ_DIR)/simdjson.o: $(SRC_DIR)/simdjson.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/orderbook.o: $(WS_SRC_DIR)/orderbook.cpp $(HEADERS)
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/test.o: $(TEST_FILE) $(HEADERS)
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean Rules
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
