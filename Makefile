#
# Makefile
#

# compiler to use
CC = clang

# flags to pass compiler
CFLAGS = -O3 -ggdb -W -Wall -Werror -Wextra

# name for executable
EXE = hangman_server

# space-separated list of header files
HDRS = hangman.h

# space-separated list of libraries, if any,
# each of which should be prefixed with -l
LIBS =

# space-separated list of source files
SRCS = hangman_server.c hangman.c

# automatically generated list of object files
OBJS = $(SRCS:.c=.o)

# default target
$(EXE): $(OBJS) $(HDRS) Makefile
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

# dependencies
$(OBJS): $(HDRS) Makefile

# housekeeping
clean:
	rm -f core $(EXE) *.o
