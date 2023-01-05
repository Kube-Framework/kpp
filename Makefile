# Transpiler's Makefile helper
# Used to prevent misusage of cmake

# Build type and directory
BUILD_DIR			=	Build
BUILD_TYPE			=	Release

# Coverage output
COVERAGE_OUTPUT		=	coverage.info

# Coverage file format to extract (see run_coverage_extract)
COVERAGE_EXTRACT	=

# Cmake arguments stack
CMAKE_ARGS			=

# Cmake generator
CMAKE_GENERATOR		=	"Ninja Multi-Config"

# CTest directory
CTEST_DIR			=	Transpiler

# Commands
RM					=	rm -rf

# General build rules
all: release

build:
	cmake -E make_directory $(BUILD_DIR)
	cmake -B $(BUILD_DIR) -H. ${CMAKE_ARGS} -G${CMAKE_GENERATOR}
	cmake --build $(BUILD_DIR) --config $(BUILD_TYPE)

release:
	$(MAKE) build BUILD_TYPE=Release

debug:
	$(MAKE) build BUILD_TYPE=Debug

# General tests rules
tests:
	$(MAKE) release CMAKE_ARGS+=-DTESTS=ON

tests_debug:
	$(MAKE) debug CMAKE_ARGS+=-DTESTS=ON

run_tests: tests
	ctest --test-dir $(BUILD_DIR)/$(CTEST_DIR) -C $(BUILD_TYPE)

run_tests_debug: tests_debug
	ctest --test-dir $(BUILD_DIR)/$(CTEST_DIR) -C $(BUILD_TYPE)

# General code coverage rules
coverage:
	$(MAKE) tests_debug CMAKE_ARGS+="-DCOVERAGE=ON"

run_coverage: coverage
	ctest --test-dir $(BUILD_DIR)/$(CTEST_DIR) -C $(BUILD_TYPE)
	lcov --directory . --capture --no-external --output-file $(COVERAGE_OUTPUT)
	lcov --remove $(COVERAGE_OUTPUT) "*/Tests/*" -o $(COVERAGE_OUTPUT)
	lcov --list $(COVERAGE_OUTPUT)

run_coverage_extract: coverage
	./$(BUILD_DIR)/Transpiler/$(COVERAGE_EXTRACT)/Tests/Debug/$(COVERAGE_EXTRACT)Tests
	lcov --directory . --capture --no-external --output-file $(COVERAGE_OUTPUT)
	lcov --remove $(COVERAGE_OUTPUT) "*/Tests/*" -o $(COVERAGE_OUTPUT)
	lcov --extract $(COVERAGE_OUTPUT) "*/$(COVERAGE_EXTRACT)/*" -o $(COVERAGE_OUTPUT)
	lcov --list $(COVERAGE_OUTPUT)

# General benchmarks rules
benchmarks:
	$(MAKE) release CMAKE_ARGS+=-DBENCHMARKS=ON

benchmarks_debug:
	$(MAKE) debug CMAKE_ARGS+=-DBENCHMARKS=ON

# Examples rules
examples:
	$(MAKE) release CMAKE_ARGS+=-DEXAMPLES=ON

examples_debug:
	$(MAKE) debug CMAKE_ARGS+=-DEXAMPLES=ON


# Transpiler
TRANSPILER_ARGS = CMAKE_ARGS+=-DTRANSPILER=ON
transpiler:
	$(MAKE) release $(TRANSPILER_ARGS)

transpiler_debug:
	$(MAKE) debug $(TRANSPILER_ARGS)

transpiler_tests:
	$(MAKE) tests $(TRANSPILER_ARGS)

run_transpiler_tests:
	$(MAKE) run_tests $(TRANSPILER_ARGS)

transpiler_tests_debug:
	$(MAKE) tests_debug $(TRANSPILER_ARGS)

run_transpiler_tests_debug:
	$(MAKE) run_tests_debug $(TRANSPILER_ARGS)

transpiler_coverage:
	$(MAKE) coverage $(TRANSPILER_ARGS)

run_transpiler_coverage:
	$(MAKE) run_coverage_extract $(TRANSPILER_ARGS) COVERAGE_EXTRACT="Transpiler"

transpiler_benchmarks:
	$(MAKE) benchmarks $(TRANSPILER_ARGS)

transpiler_benchmarks_debug:
	$(MAKE) benchmarks_debug $(TRANSPILER_ARGS)

# Cleaning rules
clean:
	$(RM) ${BUILD_DIR}

fclean: clean

# Clear and rebuild everything in release
re: clean all

.PHONY: all \
	release debug \
	examples examples_debug \
	tests tests_debug run_tests run_tests_debug \
	benchmarks benchmarks_debug \
	transpiler transpiler_debug transpiler_tests run_transpiler_tests transpiler_tests_debug run_transpiler_tests_debug transpiler_coverage run_transpiler_coverage transpiler_benchmarks transpiler_benchmarks_debug \
	clean fclean \
	re