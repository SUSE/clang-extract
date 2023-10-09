CXX=clang++
CXXFLAGS=-Wall -Wextra -fsanitize=address -g3
LDPATH=
LDFLAGS= $(LDPATH) -lclang-cpp -lclang -lLLVM -lelf
AR=llvm-ar

FILES=$(wildcard *.cpp)
HEADERS=$(wildcard *.hh)
OBJECTS=$(patsubst %.cpp, %.o, $(FILES))

libcextract.a: $(OBJECTS)
	$(AR) rc $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o *.a
