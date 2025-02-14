CXX = clang++
CXXFLAGS = -O2 -std=c++20 -I./include -I/opt/homebrew/include -I/opt/homebrew/opt/openssl/include -pthread
LDFLAGS = -L/opt/homebrew/lib -L/opt/homebrew/opt/openssl/lib
LDLIBS = -lssl -lcrypto -lboost_system -lboost_json -lcurl

SOURCES = core.cpp \
          src/coinbase.cpp \
          src/simdjson.cpp \
          src/corecomponent.cpp \
          src/websockets/coinbase_ws.cpp \
          src/websockets/crypto_ws.cpp \
          src/websocket.cpp \
          src/orderbook.cpp

OBJECTS = $(patsubst %.cpp,build/%.o,$(SOURCES))
DEPS = $(OBJECTS:.o=.d)

.PHONY: all clean

all: core

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

core: $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LDLIBS) -o $@


clean:
	rm -f core
	rm -rf build

-include $(DEPS)