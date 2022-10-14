CXX=g++
CXXFLAGS=-Wall -g -fsanitize=address -I/tmp/clang/include
LDPATH=-L/tmp/clang/lib/ -Wl,--rpath=/tmp/clang/lib/
LDFLAGS= $(LDPATH) -lclang-cpp -lclang -lLLVM

OBJECTS=Main.o PrettyPrint.o FunctionDepsFinder.o MacroDepsFinder.o EnumConstTbl.o FunctionExternalizer.o

clang-extract: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o clang-extract
