CXX = clang++
CXXFLAGS = -std=c++20 -I./include

LIBS = -lcurl

OBJS = build/core.o build/coinbase.o build/simdjson.o

all: core

dependencies: src/simdjson.o

build/simdjson.o: src/simdjson.cpp
	$(CXX) $(CXXFLAGS) -c src/simdjson.cpp -o build/simdjson.o

core: $(OBJS)
	$(CXX) $(OBJS) -o core $(LIBS)

build/core.o: core.cpp
	$(CXX) $(CXXFLAGS) -c core.cpp -o build/core.o

build/coinbase.o: src/coinbase.cpp
	$(CXX) $(CXXFLAGS) -c src/coinbase.cpp -o build/coinbase.o

clean:
	rm -f $(OBJS) core
