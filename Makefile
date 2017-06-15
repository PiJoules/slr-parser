CPP = g++
STD = c++11
CPPFLAGS = -Wall -Werror -std=$(STD)

SOURCES = lexer.cpp
OBJS = $(SOURCES:.cpp=.o)

EXE_FILES = test_lexer.cpp 
EXE_OUTPUTS = $(EXE_FILES:.cpp=.out)

tests: $(OBJS) test_lexer

.PHONY: tests

# Object files from cpp files
%.o: %.cpp 
	$(CPP) $(CPPFLAGS) -c $< -o $@

compile: $(OBJS)
compile_exes: $(EXE_OUTPUTS)

# Executable binaries from cpp files
%.out: %.cpp
	$(CPP) $(CPPFLAGS) $< $(OBJS) -o $@

test_lexer: compile_exes
	./test_lexer.out

clean:
	rm -rf *.o *.out
