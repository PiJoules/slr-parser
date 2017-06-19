CPP = g++
STD = c++11
CPPFLAGS = -Wall -Werror -std=$(STD)

SOURCES = lexer.cpp \
		  parser.cpp \
		  lang_utils.cpp \
		  lang_rules.cpp
OBJS = $(SOURCES:.cpp=.o)

EXE_FILES = test_lexer.cpp \
			test_table_generation.cpp \
			dump_lang.cpp
EXE_OUTPUTS = $(EXE_FILES:.cpp=.out)

test: $(SOURCES) $(OBJS) $(EXE_FILES) test_lexer test_table_generation

.PHONY: test

# Object files from cpp files
%.o: %.cpp 
	$(CPP) $(CPPFLAGS) -O2 -c $< -o $@

compile: $(OBJS) clean_exes $(EXE_OUTPUTS)

# Executable binaries from cpp files
%.out: %.cpp
	$(CPP) $(CPPFLAGS) $< $(OBJS) -o $@

clean_exes:
	rm -rf *.out 

test_lexer: clean_exes test_lexer.out
	./test_lexer.out
	valgrind ./test_lexer.out || echo 'Valgrind not available'

test_table_generation: clean_exes test_table_generation.out
	./test_table_generation.out
	valgrind ./test_table_generation.out || echo 'Valgrind not available'

clean:
	rm -rf *.o *.out
