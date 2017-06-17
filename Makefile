CPP = g++
STD = c++11
CPPFLAGS = -Wall -Werror -std=$(STD)

SOURCES = lang.h \
		  lexer.cpp \
		  parser.cpp
OBJS = $(SOURCES:.cpp=.o)

EXE_FILES = test_lexer.cpp 
EXE_OUTPUTS = $(EXE_FILES:.cpp=.out)

tests: $(SOURCES) $(OBJS) $(EXE_FILES) test_lexer

.PHONY: tests

# Object files from cpp files
%.o: %.cpp 
	$(CPP) $(CPPFLAGS) -c $< -o $@

compile: $(OBJS)
compile_exes: $(EXE_OUTPUTS)

# Executable binaries from cpp files
%.out: %.cpp $(OBJS)
	$(CPP) $(CPPFLAGS) $^ -o $@

test_lexer: test_lexer.out
	./test_lexer.out
	valgrind ./test_lexer.out || echo 'Valgrind not available'

clean:
	rm -rf *.o *.out
