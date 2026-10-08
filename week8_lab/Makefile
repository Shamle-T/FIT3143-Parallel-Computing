BUILD_DIR := build
CC ?= gcc
MPICC ?= mpicc
CFLAGS := -std=c11 -O3 -Wall -Wextra -Wpedantic
LDLIBS := -lm

.PHONY: all clean verify benchmark

all: $(BUILD_DIR)/task1_serial $(BUILD_DIR)/task1 $(BUILD_DIR)/task2 $(BUILD_DIR)/task3_benchmark

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/task1_serial: task1_serial.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BUILD_DIR)/task1: task1.c | $(BUILD_DIR)
	$(MPICC) $(CFLAGS) $< -o $@ $(LDLIBS)

$(BUILD_DIR)/task2: task2.c | $(BUILD_DIR)
	$(MPICC) $(CFLAGS) -fopenmp $< -o $@ $(LDLIBS)

$(BUILD_DIR)/task3_benchmark: task3_benchmark.C | $(BUILD_DIR)
	$(CC) $(CFLAGS) -x c -fopenmp $< -o $@ $(LDLIBS)

verify: all
	./scripts/verify_correctness.sh

benchmark: all
	./scripts/run_benchmarks.sh

clean:
	rm -rf $(BUILD_DIR)
