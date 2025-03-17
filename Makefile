# TO DO : Make the Makefile cleaner

CC = gcc

AR = ar rcs

CFLAGS = -Wall -std=c99 -Werror

SRC = src/cnm.c

OBJ = $(SRC:.c=.o)

LIB = libcnm.a

# Test File
TEST_SRC = example/test.c
TEST_EXE = test_program

# Rule to create static library
$(LIB): $(OBJ)
	$(AR) $(LIB) $(OBJ)

# Rule to compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(LIB) $(TEST_SRC)
	$(CC) $(CFLAGS) -o $(TEST_EXE) $(TEST_SRC) -L. -l:libcnm.a

# Clean rule to remove object files and library
clean:
	rm -f $(OBJ) $(LIB) $(TEST_EXE)

# Phony targets (not actual files)
.PHONY: clean