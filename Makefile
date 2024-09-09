# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++11 -Wall -Wextra -I /usr/include/boost

# Linker flags
LDFLAGS := -L /usr/lib -lboost_system -lboost_filesystem -lboost_program_options

# Source files
SRCS := ship.cpp utils.cpp vm_operations.cpp container_operations.cpp 

# Object files
OBJS := $(SRCS:.cpp=.o)

# Dependency files
DEPS := $(SRCS:.cpp=.d)

# Output executable
TARGET := ship

# Base directory path
# BASE_DIR := /var/lib/ship
BASE_DIR := /home/ship

# Directory paths
DIRS := $(BASE_DIR) $(BASE_DIR)/images $(BASE_DIR)/images/iso-images $(BASE_DIR)/images/disk-images $(BASE_DIR)/settings $(BASE_DIR)/settings/ctr-settings $(BASE_DIR)/settings/vm-settings

# File paths
FILES := $(BASE_DIR)/settings/general_settings.ini

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

# Rule for compiling .cpp files into .o object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Rule for creating dependency files
%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@

# Include dependency files
-include $(DEPS)

# Clean rule
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

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



