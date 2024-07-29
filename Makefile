# Makefile for compiling ship

# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++11 -Wall -Wextra -I /usr/include/boost

# Linker flags
LDFLAGS := -L /usr/lib -lboost_system -lboost_filesystem

# Source files
SRCS := ship.cpp utils.cpp vm_operations.cpp container_operations.cpp 

# Object files
OBJS := $(SRCS:.cpp=.o)

# Output executable
TARGET := ship

# Build target
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule for compiling .cpp files into .o object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

