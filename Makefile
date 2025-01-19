CXX = clang++
CXXFLAGS = -std=c++20 -I./include -I/opt/homebrew/include -I/opt/homebrew/opt/openssl/include -pthread
LDFLAGS = -L/opt/homebrew/lib -L/opt/homebrew/opt/openssl/lib
LDLIBS = -lssl -lcrypto -lboost_system -lboost_json -lcurl

# Source files
SOURCES = core.cpp \
          src/coinbase.cpp \
          src/simdjson.cpp \
          src/corecomponent.cpp \
          src/websocket.cpp \
          src/orderbook.cpp

# Object files (all in build/)
OBJS = $(patsubst %.cpp,build/%.o,$(notdir $(SOURCES)))

# Main target
all: core

.PHONY: all clean

# Create build directory
build:
	mkdir -p build

# Pattern rule for object files in root directory
build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pattern rule for object files from src directory
build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Main target linking
core: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# Clean target
clean:
	rm -f core
	rm -rf build