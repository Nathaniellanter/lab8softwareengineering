# 1. Compiler and Flags
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -O2 -g

# 2. Target Executable Name
TARGET   = my_program

# 3. Source and Object Files
SRCS     = string_calculator.cpp
OBJS     = $(SRCS:.cpp=.o)

# 4. Default Target (runs when you just type 'make')
all: $(TARGET)

# 5. Rule to link object files into the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# 6. Pattern rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 7. Clean target to delete generated build files
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)
