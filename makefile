# compile all the c files from src directory, place the .o files in obj directory, and have the main exe in the root directory
# make clean to remove all .o files and the main exe
# make run to run the main exe
# make all to compile and run the main exe

# Compiler
CC = clang

# Compiler flags
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -g

# Source files
SRC = $(wildcard src/*.c)

# Object files
OBJ = $(SRC:src/%.c=obj/%.o)

# Main executable
MAIN = main

# Compile all the object files and link them to the main executable
all: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(MAIN)

# Compile all the object files
obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the main executable
run: all
	./$(MAIN)

# Remove all the object files and the main executable
clean:
	rm -f $(OBJ) $(MAIN)

