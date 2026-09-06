# Makefile for building and running the TaskManager project
# This Makefile was created with assistance from xAI's Grok 3 model for guidance on structure, commands, and debugging.

# Compiler
JC = javac

# Source files
SOURCES = MyPriorityQueue.java MyUser.java MyTask.java MinHeap.java TaskManager.java

# Output directory
OUT = .

# Default target
all: build

# Build the project
build:
	$(JC) -d $(OUT) $(SOURCES)

# Run the program with arguments
run:
	@java -cp $(OUT) TaskManager $(ARGS)

# Clean compiled files
clean:
	rm -f *.class

# Generate Javadoc
javadoc:
	javadoc -d doc $(SOURCES)

.PHONY: all build run clean javadoc