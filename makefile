# Compiler and flags
CC = gcc
CFLAGS = -I$(HEADERS_DIR)

# Directories
SRC_DIR = src
HEADERS_DIR = $(SRC_DIR)/headers
SOURCES_DIR = $(SRC_DIR)/sources
OBJ_DIR = obj

# Source and object files
SOURCES = $(wildcard $(SOURCES_DIR)/*.c) $(SRC_DIR)/main.c $(SRC_DIR)/random_permutation.c
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

# Executable name
TARGET = main

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compiling object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Run the executable
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run