CPPC                 = ${COMPILER}
STANDARD_FLAGS       = -std=c++0x
WARN_AS_ERRORS_FLAGS = -pedantic-errors\
                       -Wall\
                       -Wpedantic\
                       -Wno-unused-parameter
DEBUG_FLAGS          = -g3
OPT_FLAGS            =
NO_LINKER_FLAGS      = -c
CPPFLAGS             = $(OPT_FLAGS) $(DEBUG_FLAGS) $(NO_LINKER_FLAGS) $(STANDARD_FLAGS) \
                       $(WARN_AS_ERRORS_FLAGS)

# Source files, headers, etc.:
ROOT     = $(CURDIR)
INCLUDES = -I$(ROOT)/include -I${BOOST_INCLUDES} -I src/generated

SRCS     := $(wildcard src/*.cpp) src/generated/mdw/formula/parse/yacc/YaccParser.cpp


OBJS := $(SRCS:.cpp=.o)
OBJS := $(subst src,obj,$(OBJS))

LIBS =
MAIN = libmdwFormula.a # static library


all:    $(MAIN)
	@echo $(MAIN) has been compiled!

src/generated/mdw/formula/parse/flex/Lexer.hpp: grammar/Lexer.ll
	echo "Generating flex grammar"
	mkdir -p src/generated/mdw/formula/parse/flex/
	flex -o src/generated/mdw/formula/parse/flex/Lexer.hpp -P formula grammar/Lexer.ll

src/generated/mdw/formula/parse/yacc/YaccParser.cpp: grammar/YaccParser.yy
	echo "Generating bison grammar"
	mkdir -p src/generated/mdw/formula/parse/yacc/
	bison -o src/generated/mdw/formula/parse/yacc/YaccParser.cpp -p formula -d grammar/YaccParser.yy

$(MAIN): src/generated/mdw/formula/parse/flex/Lexer.hpp src/generated/mdw/formula/parse/yacc/YaccParser.cpp $(OBJS)
	ar -r $(MAIN) $(OBJS) $(LIBS)

obj/%.o: src/%.cpp
	mkdir -p obj
	mkdir -p obj/generated/mdw/formula/parse/yacc
	$(CPPC) $(CPPFLAGS) $(INCLUDES) -fPIC -c $< -o $@

depend: $(SRCS)
	makedepend $(INCLUDES) $^all: lib test

clean:
	rm -rf src/generated
	rm -rf obj
	rm -f libmdwFormula.a

