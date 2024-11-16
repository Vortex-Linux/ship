# Compiler
CXX := g++

# Directory paths
SRC_DIR := src
INCLUDE_DIR := include
OBJ_DIR := obj
BUILD_DIR := build

# Compiler flags
CXXFLAGS := -std=c++11 -Wall -Wextra -I/usr/include/boost -I $(INCLUDE_DIR)

# Linker flags
LDFLAGS := -L /usr/lib -lboost_system -lboost_filesystem -lboost_program_options

# Base directory path
BASE_DIR := /home/ship

# Automatically find source files in the src directory
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Dependency files
DEPS := $(OBJS:.o=.d)

# Output executable
TARGET := $(BUILD_DIR)/ship

# Directory paths to ensure they exist
DIRS := $(BASE_DIR) $(BASE_DIR)/images $(BASE_DIR)/images/iso-images $(BASE_DIR)/images/disk-images \
        $(BASE_DIR)/settings $(BASE_DIR)/settings/ctr-settings $(BASE_DIR)/settings/vm-settings

# File paths
FILES := $(BASE_DIR)/settings/general_settings.ini

# Create necessary directories
$(shell mkdir -p $(OBJ_DIR) $(BUILD_DIR))

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

# Rule for compiling .cpp files into .o object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MP

# Include dependency files (only if they exist)
-include $(wildcard $(DEPS))

# Clean rule
clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

# Install rule
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/
	@for dir in $(DIRS); do \
		if [ ! -d $$dir ]; then \
			sudo install -d -m 777 $$dir; \
		fi \
	done
	@for file in $(FILES); do \
		if [ ! -f $$file ]; then \
			sudo touch $$file; \
			sudo chmod 666 $$file; \
		fi \
	done

.PHONY: all clean install
