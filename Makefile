CXX=clang++ #/tmp/clang/usr/local/bin/clang++
CXXFLAGS=-Wall -g -fsanitize=address # -I/tmp/clang/include
LDPATH=#-L/tmp/clang/lib/ -Wl,--rpath=/tmp/clang/lib/
LDFLAGS= $(LDPATH) -lclang-cpp -lclang -lLLVM

PYTHON=python -B

OBJECTS= \
	Main.o \
	PrettyPrint.o \
	FunctionDepsFinder.o \
	EnumConstTbl.o \
	SymbolExternalizer.o \
	ArgvParser.o \
	FunctionExternalizeFinder.o \
	MacroWalker.o \
	Passes.o

clang-extract: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

check: testsuite
	$(MAKE) -C $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o clang-extract
