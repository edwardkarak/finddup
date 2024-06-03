CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = $(shell pkg-config --libs openssl)
INCLUDEFLAGS = $(shell pkg-config --cflags openssl)
OUTPUT = finddup
SOURCES = src/main.cpp src/util.cpp src/finddup.cpp
BUILD_DIR = build

# Object files corresponding to source files, placed in BUILD_DIR
OBJECTS = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

# Ensure build directory exists
$(shell mkdir -p $(BUILD_DIR))

all: $(OUTPUT)

# Link object files to create the output
$(OUTPUT): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) $(OBJECTS) -o $(OUTPUT) $(LDFLAGS)

# Compile each source file into an object file
$(BUILD_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDEFLAGS) -c $< -o $@

# Clean up object files and output
clean:
	rm -f $(OUTPUT) $(BUILD_DIR)/*.o

.PHONY: all clean
