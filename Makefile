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

