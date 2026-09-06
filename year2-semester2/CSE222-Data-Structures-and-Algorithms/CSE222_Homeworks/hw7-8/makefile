# Define directories
SRC = src
BUILD = build
DOC = doc

# Define test classes (without .java extension)
TEST_CLASSES = DSA.Sorting.GTUSelectSortTest DSA.Sorting.GTUInsertSortTest DSA.Sorting.GTUQuickSortTest \
               DSA.Graphs.MatrixGraphTest DSA.Graphs.AdjacencyVectTest DSA.Graphs.GCA.GreedyGCATest \
               DSA.Graphs.GCA.GCASolutionTest Main.TestScenarioTest DSA.Graphs.GCA.GreedyVertexTest

# Default target
all: clean build doc

# Create necessary directories if they don't exist
dirs:
	mkdir -p $(BUILD) $(DOC)

# Collect all .java files into sources.txt
collect:
	find $(SRC) -name "*.java" > sources.txt

# Build the project (depends on collect and directory creation)
build: collect dirs
	javac -d $(BUILD) -sourcepath $(SRC) @sources.txt

# Run the program (depends on build)
run: build
	java -cp $(BUILD) Main.Main $(ARGS)

# Generate Javadoc documentation (depends on collect and directory creation)
doc: collect dirs
	javadoc -d $(DOC) -sourcepath $(SRC) @sources.txt

# Run unit tests (depends on build)
test: build
	@echo "Running unit tests..."
	@for test in $(TEST_CLASSES); do \
		echo "Running $$test..."; \
		java -cp $(BUILD) $$test; \
		echo ""; \
	done

# Clean up
clean:
	rm -rf $(BUILD) sources.txt $(DOC)

# Phony targets to avoid conflicts with file names
.PHONY: all clean collect build run doc dirs test