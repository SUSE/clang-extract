CXX=clang++
CXXFLAGS=-Wall -g
LDFLAGS=-lclang-cpp -lclang -lLLVM

OBJECTS=Main.o PrettyPrint.o FunctionDepsFinder.o MacroDepsFinder.o EnumConstTbl.o

clang-extract: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o clang-extract
