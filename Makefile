SRC = builtin.c  exec.c  jshcore.c  main.c
PROGRAM = jsh
OBJ = $(SRC:.c=.o)
CC = gcc
INCLUDE =
LINKPARAM =
CFLAGS = -O0 -Wall -Wextra -Wno-unused-parameter -g
RM_FLAGS =

: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(LINKPARAM) $(OBJ)

%.o: %.c
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

clean:
	rm $(RM_FLAGS) $(OBJ) $(PROGRAM)
