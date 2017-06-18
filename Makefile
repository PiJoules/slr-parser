CPP = g++
STD = c++11
CPPFLAGS = -Wall -Werror -std=$(STD)

SOURCES = lang.h \
		  lexer.cpp \
		  parser.cpp \
		  lang_utils.cpp
OBJS = $(SOURCES:.cpp=.o)

EXE_FILES = test_lexer.cpp 
EXE_OUTPUTS = $(EXE_FILES:.cpp=.out)

tests: $(SOURCES) $(OBJS) $(EXE_FILES) test_lexer test_table_generation

.PHONY: tests

# Object files from cpp files
%.o: %.cpp 
	$(CPP) $(CPPFLAGS) -O2 -c $< -o $@

compile: $(OBJS)
compile_exes: $(EXE_OUTPUTS)

# Executable binaries from cpp files
%.out: %.cpp
	$(CPP) $(CPPFLAGS) $< $(OBJS) -o $@

test_lexer: test_lexer.out
	./$<
	valgrind ./$< || echo 'Valgrind not available'

test_table_generation: test_table_generation.out
	./$<
	valgrind ./$< || echo 'Valgrind not available'

clean:
	rm -rf *.o *.out
