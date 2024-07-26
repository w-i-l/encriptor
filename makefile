# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Directories
SRC_DIR = src
HEADERS_DIR = $(SRC_DIR)/headers
SOURCES_DIR = $(SRC_DIR)/sources
OBJ_DIR = obj

# Source and object files
SOURCES = $(wildcard $(SOURCES_DIR)/*.c) $(SRC_DIR)/main.c
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(patsubst $(SOURCES_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES)))

# Executable name and path
TARGET_DIR = $(SRC_DIR)
TARGET = $(TARGET_DIR)/main

# Default target
all: $(TARGET)

# Linking the executable
$(TARGET): $(OBJECTS)
	@echo "Linking: $(OBJECTS)"
	$(CC) $(OBJECTS) -o $(TARGET)

# Compiling object files from src directory (for main.c)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compiling object files from sources directory
$(OBJ_DIR)/%.o: $(SOURCES_DIR)/%.c $(HEADERS_DIR)/%.h
	@mkdir -p $(@D)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Run the executable
run: $(TARGET)
	./$(TARGET)

# Debug info
debug:
	@echo "Sources: $(SOURCES)"
	@echo "Objects: $(OBJECTS)"

.PHONY: all clean run debug