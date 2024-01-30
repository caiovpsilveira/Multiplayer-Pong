CC = gcc
CFLAGS = -Wall -Wextra -Isrc
LDFLAGS = -lm -lGL -lglfw

SRC_DIR = src
BUILD_DIR = build

# Get a list of all .c files in the src directory
SRCS := $(wildcard $(SRC_DIR)/*.c)

# Generate a list of corresponding .o files in the build directory
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Names of the final executables
CLIENT_TARGET = client.out
SERVER_TARGET = server.out

all: client server
debug: CFLAGS += -DDEBUG
debug: all

# Rule to compile each .c file into a .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link client executable
client: $(BUILD_DIR)/client.o $(filter-out $(BUILD_DIR)/server.o, $(OBJS))
	$(CC) $^ -o $(CLIENT_TARGET) $(LDFLAGS)

# Rule to link server executable
server: $(BUILD_DIR)/server.o $(filter-out $(BUILD_DIR)/client.o, $(OBJS))
	$(CC) $^ -o $(SERVER_TARGET) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(CLIENT_TARGET) $(SERVER_TARGET)

.PHONY: all client server clean