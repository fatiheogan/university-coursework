# Makefile for Planetary System project

# Variables
JAVAC = javac
JAVA = java
SOURCES = SensorData.java Node.java PlanetarySystem.java
MAIN_CLASS = PlanetarySystem
CLASS_FILES = $(SOURCES:.java=.class)

# Default target: compile all files
all: compile

# Compile Java files
compile: $(SOURCES)
	$(JAVAC) $(SOURCES)

# Run the main class
run: compile
	$(JAVA) $(MAIN_CLASS)

# Clean up compiled files
clean:
	rm -f *.class

# Phony targets (not actual files)
.PHONY: all compile run clean