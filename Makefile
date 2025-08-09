# Makefile for CodeGeneratorBot
# Self-Sustaining Autonomous Code Generator

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
LDFLAGS = -pthread

# Debug flags (uncomment for debugging)
# CXXFLAGS += -g -DDEBUG

# Directories
SRCDIR = .
OBJDIR = obj
BINDIR = bin

# Source files
SOURCES = main.cpp CodeGeneratorBot.cpp
OBJECTS = $(SOURCES:%.cpp=$(OBJDIR)/%.o)
TARGET = $(BINDIR)/codegen_bot

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)
	@mkdir -p bot_workspace
	@mkdir -p bot_workspace/generated
	@mkdir -p bot_workspace/compiled
	@mkdir -p bot_workspace/logs

# Build the main executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build completed successfully!"
	@echo "Run './$(TARGET) --help' for usage information"

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Install target (optional)
install: $(TARGET)
	@echo "Installing CodeGeneratorBot..."
	sudo cp $(TARGET) /usr/local/bin/codegen_bot
	@echo "Installation completed. You can now run 'codegen_bot' from anywhere."

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)
	@echo "Clean completed."

# Clean everything including generated files and logs
distclean: clean
	@echo "Performing deep clean..."
	rm -rf bot_workspace
	rm -f bot_config.txt
	rm -f bot_activity.log
	rm -f *.log
	@echo "Deep clean completed."

# Run the bot (for convenience)
run: $(TARGET)
	./$(TARGET)

# Run the bot in auto-start mode
run-auto: $(TARGET)
	./$(TARGET) --auto-start

# Run demo mode
demo: $(TARGET)
	./$(TARGET) --demo

# Debug build
debug: CXXFLAGS += -g -DDEBUG -O0
debug: clean $(TARGET)

# Check if required tools are available
check-deps:
	@echo "Checking dependencies..."
	@which g++ > /dev/null || (echo "Error: g++ not found. Please install build-essential." && exit 1)
	@g++ --version | head -1
	@echo "Dependencies OK."

# Show help
help:
	@echo "CodeGeneratorBot Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build the bot (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  distclean  - Remove all generated files"
	@echo "  run        - Build and run the bot"
	@echo "  run-auto   - Build and run the bot in auto-start mode"
	@echo "  demo       - Build and run demo mode"
	@echo "  debug      - Build with debug symbols"
	@echo "  install    - Install to /usr/local/bin"
	@echo "  check-deps - Check if required tools are available"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build the bot"
	@echo "  make run                # Build and run with user confirmation"
	@echo "  make run-auto           # Build and run automatically"
	@echo "  make demo               # Show demo of original functionality"
	@echo "  make debug              # Build with debugging enabled"
	@echo "  make install            # Install system-wide"

# Test build (compile but don't run)
test-build: $(TARGET)
	@echo "Test build completed successfully!"
	@echo "Bot executable created: $(TARGET)"
	@echo "To run: ./$(TARGET)"

# Package for distribution
package: clean
	@echo "Creating distribution package..."
	tar -czf codegen_bot_v1.0.tar.gz *.cpp *.h Makefile README.md 2>/dev/null || tar -czf codegen_bot_v1.0.tar.gz *.cpp *.h Makefile
	@echo "Package created: codegen_bot_v1.0.tar.gz"

# Phony targets
.PHONY: all clean distclean run run-auto demo debug check-deps help test-build package install directories

# Dependencies (automatically generated)
$(OBJDIR)/main.o: main.cpp CodeGeneratorBot.h
$(OBJDIR)/CodeGeneratorBot.o: CodeGeneratorBot.cpp CodeGeneratorBot.h