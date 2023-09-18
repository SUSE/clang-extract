CXX=clang++
CXXFLAGS=-Wall -Wextra -fsanitize=address -g3
LDPATH=
LDFLAGS= $(LDPATH)-lelf

PYTHON=python -B

FILES=$(wildcard *.cpp)
HEADERS=$(wildcard *.hh)
OBJECTS=$(patsubst %.cpp, %.o, $(FILES))

clang-extract: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

inline: InlineAnalysis.o ElfCXX.o IpaClonesParser.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lelf

check: clang-extract
	$(MAKE) -C testsuite

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o clang-extract
	$(MAKE) -C testsuite clean
