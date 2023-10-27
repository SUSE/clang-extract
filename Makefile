include Makefile.common

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

FILES=$(wildcard *.cpp)
HEADERS=$(wildcard *.hh)
OBJECTS=$(patsubst %.cpp, %.o, $(FILES))

.PHONY: libcextract/libcextract.a check clean

all: clang-extract inline

clang-extract: Main.o libcextract/libcextract.a
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

inline: Inline.o libcextract/libcextract.a
	$(CXX) $(CXXFLAGS) -o $@ $^ -lelf

libcextract/libcextract.a:
	$(MAKE) -C libcextract

check: clang-extract
	$(MAKE) -C testsuite

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o clang-extract inline
	$(MAKE) -C testsuite clean
	$(MAKE) -C libcextract clean
