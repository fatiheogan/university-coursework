# Makefile for SpellChecker project

# Variables
JAVAC = javac
JAVA = java
SRC_DIR = src
MAIN_CLASS = SpellChecker
SOURCES = $(wildcard $(SRC_DIR)/*.java)
CLASSES = $(patsubst $(SRC_DIR)/%.java,%.class,$(SOURCES))

# Classpath and main class (no package)
CLASS_PATH = .
MAIN = $(MAIN_CLASS)

# Default target
all: build

# Build (compile) all Java files
build:
	$(JAVAC) -d . $(SOURCES)

# Run the SpellChecker program
run: build
	$(JAVA) -cp $(CLASS_PATH) $(MAIN)

# Clean compiled files
clean:
	rm -f $(CLASS_PATH)/*.class

# Phony targets
.PHONY: all build run clean