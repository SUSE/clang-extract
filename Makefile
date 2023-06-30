CXX=clang++ #/tmp/clang/usr/local/bin/clang++
CXXFLAGS=-Wall -g -fsanitize=address # -I/tmp/clang/include
LDPATH=#-L/tmp/clang/lib/ -Wl,--rpath=/tmp/clang/lib/
LDFLAGS= $(LDPATH) -lclang-cpp -lclang -lLLVM

PYTHON=python -B

FILES=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp, %.o, $(FILES))

clang-extract: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

check: clang-extract
	$(MAKE) -C testsuite

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o clang-extract
