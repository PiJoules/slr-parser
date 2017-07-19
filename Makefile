CPP = g++-4.9
STD = c++11

CPPFLAGS = -Wall -Werror -std=$(STD) -Wfatal-errors $(MACROS)

MEMCHECK = valgrind --error-exitcode=1 --leak-check=full

SOURCES = lexer.cpp \
		  parser.cpp \
		  lang_lexer.cpp \
		  lang_parser.cpp \
		  lang_utils.cpp \
		  lang_rules.cpp \
		  lang_nodes.cpp
OBJS = $(SOURCES:.cpp=.o)

TEST_FILES = test_lexer.cpp \
			 test_table_generation.cpp \
			 test_parser.cpp

EXE_FILES = $(TEST_FILES) \
			dump_lang.cpp \
			#lang.cpp
EXE_OUTPUTS = $(EXE_FILES:.cpp=.out)

test: compile_clean test_lexer test_table_generation test_parser 

.PHONY: test

# Object files from cpp files
%.o: %.cpp 
	$(CPP) $(CPPFLAGS) -O2 -c $< -o $@

compile: $(OBJS) $(EXE_OUTPUTS)
compile_objs: $(OBJS)
compile_clean: $(OBJS) clean_exes $(EXE_OUTPUTS)

# Executable binaries from cpp files
%.out: %.cpp
	$(CPP) $(CPPFLAGS) $< $(OBJS) -o $@

clean_exes:
	rm -rf $(EXE_OUTPUTS)

test_lexer: $(OBJS) clean_exes test_lexer.out
	./test_lexer.out
	if [ -x "$$(command -v valgrind)" ]; then $(MEMCHECK) ./test_lexer.out || (echo "memory leak"; exit 1); fi

test_table_generation: $(OBJS) clean_exes test_table_generation.out
	./test_table_generation.out
	if [ -x "$$(command -v valgrind)" ]; then $(MEMCHECK) ./test_table_generation.out || (echo "memory leak"; exit 1); fi 

test_parser: $(OBJS) clean_exes test_parser.out
	./test_parser.out
	if [ -x "$$(command -v valgrind)" ]; then $(MEMCHECK) ./test_parser.out || (echo "memory leak"; exit 1); fi

dump_lang: $(OBJS) clean_exes dump_lang.out
	./dump_lang.out

clean:
	rm -rf *.o *.out
