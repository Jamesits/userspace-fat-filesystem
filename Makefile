SRC = $(wildcard *.c)
PROGRAM = fs
OBJ = $(SRC:.c=.o)
CC = clang
INCLUDE =
LINKPARAM =
CFLAGS = -O0 -Wall -Wextra -Wno-unused-parameter -g3 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -D_GNU_SOURCE 
RM_FLAGS =

: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(LINKPARAM) $(OBJ)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

clean:
	rm $(RM_FLAGS) $(OBJ) $(PROGRAM)
