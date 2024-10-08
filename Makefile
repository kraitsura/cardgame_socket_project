# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -pthread

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SERVER_SRCS = $(SRC_DIR)/TrackerServer.cpp $(SRC_DIR)/Tracker.cpp
CLIENT_SRCS = $(SRC_DIR)/PlayerClient.cpp
COMMON_SRCS = $(SRC_DIR)/Utils.cpp

# Object files
SERVER_OBJS = $(SERVER_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
CLIENT_OBJS = $(CLIENT_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
COMMON_OBJS = $(COMMON_SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Executables
SERVER_TARGET = $(BIN_DIR)/TrackerServer
CLIENT_TARGET = $(BIN_DIR)/PlayerClient

# Phony targets
.PHONY: all clean server client

# Default target
all: server client

# Server target
server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Client target
client: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Object file compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create directories
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Include dependencies
-include $(SERVER_OBJS:.o=.d)
-include $(CLIENT_OBJS:.o=.d)
-include $(COMMON_OBJS:.o=.d)

# Generate dependency files
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@$(CXX) $(CXXFLAGS) -MM -MT $(@:.d=.o) $< > $@