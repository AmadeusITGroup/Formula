CPPC                 = ${COMPILER}
STANDARD_FLAGS       = -std=c++0x
WARN_AS_ERRORS_FLAGS = -pedantic-errors\
                       -Wall\
                       -Wextra\
                       -Wno-unused-parameter\
                       -Wconversion
DEBUG_FLAGS          = -g3
OPT_FLAGS            =
NO_LINKER_FLAGS      = -c
CPPFLAGS             = $(OPT_FLAGS) $(DEBUG_FLAGS) $(NO_LINKER_FLAGS) $(STANDARD_FLAGS) \
                       $(WARN_AS_ERRORS_FLAGS)

# Source files, headers, etc.:
ROOT     = $(CURDIR)
INCLUDES = -I$(ROOT)/../include -I${BOOST_INCLUDES} -I${GMOCK_INCLUDES} -I${GTEST_INCLUDES}

SRCS     := $(wildcard src/*.cpp)

OBJS     := $(SRCS:.cpp=.o)
OBJS     := $(subst src,obj,$(OBJS))

LIBS = ../libmdwFormula.a /projects/mwdeldev/Gtest/15-0-0-4/lib/x86_64-2.6.32-v2/Debug/libgtest.a /opt/1A/toolchain/x86_64-2.6.32-v2/lib/libpthread.a /opt/1A/toolchain/x86_64-2.6.32-v2/lib/librt.a /opt/1A/toolchain/x86_64-2.6.32-v2/lib/libstdc++.a
MAIN = test


all:    $(MAIN)
	@echo $(MAIN) has been compiled!

$(MAIN): $(OBJS)
	$(CPPC) -pie -o $(MAIN) $(OBJS) $(LIBS)

obj/%.o: src/%.cpp
	mkdir -p obj
	$(CPPC) $(CPPFLAGS) $(INCLUDES) -fPIC -c $< -o $@

depend: $(SRCS)
	makedepend $(INCLUDES) $^all: lib test

clean:
	rm -rf obj
	rm -f test

