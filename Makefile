CXX ?= clang++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O3 -Iinclude
BIN_DIR = bin

TARGETS = $(BIN_DIR)/demo \
          $(BIN_DIR)/test-runner \
          $(BIN_DIR)/test-avl-tree \
          $(BIN_DIR)/test-red-black-tree \
          $(BIN_DIR)/test-b-tree \
          $(BIN_DIR)/benchmark

.PHONY: all clean test benchmark run-demo help

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/demo: src/main.cpp include/search-trees/*.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) src/main.cpp -o $@

$(BIN_DIR)/test-runner: tests/test-runner.cpp include/search-trees/*.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) tests/test-runner.cpp -o $@

$(BIN_DIR)/test-avl-tree: tests/test-avl-tree.cpp include/search-trees/*.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) tests/test-avl-tree.cpp -o $@

$(BIN_DIR)/test-red-black-tree: tests/test-red-black-tree.cpp include/search-trees/*.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) tests/test-red-black-tree.cpp -o $@

$(BIN_DIR)/test-b-tree: tests/test-b-tree.cpp include/search-trees/*.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) tests/test-b-tree.cpp -o $@

$(BIN_DIR)/benchmark: tests/benchmark.cpp include/search-trees/*.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) tests/benchmark.cpp -o $@

test: $(BIN_DIR)/test-runner
	./$(BIN_DIR)/test-runner

test-all: $(TARGETS)
	./$(BIN_DIR)/test-avl-tree
	./$(BIN_DIR)/test-red-black-tree
	./$(BIN_DIR)/test-b-tree
	./$(BIN_DIR)/test-runner

benchmark: $(BIN_DIR)/benchmark
	./$(BIN_DIR)/benchmark

run-demo: $(BIN_DIR)/demo
	./$(BIN_DIR)/demo

clean:
	rm -rf $(BIN_DIR)

help:
	@echo "Balanced Search Trees Makefile targets:"
	@echo "  make all        - Build all executables (demo, tests, benchmark)"
	@echo "  make test       - Build and run the unified test suite"
	@echo "  make test-all   - Build and run every individual unit test"
	@echo "  make benchmark  - Build and execute performance benchmarks"
	@echo "  make run-demo   - Build and start interactive CLI demonstration"
	@echo "  make clean      - Remove build artifacts"
