# Define the compiler and the flags
CC = gcc
CFLAGS = -Wall -I/opt/homebrew/include -I./include -pthread
# For debug add -g -fsanitize=address
# lldb ./executable/vector_db_server
# breakpoint set -n malloc_error_break
# run
# bt
LDFLAGS = -L/opt/homebrew/lib -lmicrohttpd -lcjson -pthread

# Define the target executable and directory
TARGET_DIR = executable
TARGET = $(TARGET_DIR)/vector_db_server

# Define the source files
SRCS = src/vector_database.c src/get_handler.c src/post_handler.c src/put_handler.c src/delete_handler.c src/compare_handler.c src/main.c src/kdtree.c

# Define the object files with directory prefix
OBJS = $(addprefix $(TARGET_DIR)/, $(notdir $(SRCS:.c=.o)))

# Default rule to build the target
all: $(TARGET)

# Rule to create the target directory if it does not exist
$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

# Rule to build the target executable from object files
$(TARGET): $(OBJS) | $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Rule to compile source files into object files in the target directory
$(TARGET_DIR)/%.o: src/%.c | $(TARGET_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up only the object files
clean:
	rm -f $(OBJS)

# Clean up all generated files (object files and executable)
clean-all:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean clean-all
