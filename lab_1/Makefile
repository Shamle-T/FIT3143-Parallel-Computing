CC = gcc
CFLAGS = -std=c11 -O3 -Wall -Wextra -pedantic
LDLIBS = -lm
PTHREAD_FLAGS = -pthread
OPENMP_FLAGS = -fopenmp
BIN_DIR = bin
TASK1 = $(BIN_DIR)/task1.exe
TASK2 = $(BIN_DIR)/task2.exe
TASK3 = $(BIN_DIR)/task3.exe

.PHONY: all clean task1 task2 task3

all: $(TASK1) $(TASK2) $(TASK3)

task1: $(TASK1)

task2: $(TASK2)

task3: $(TASK3)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TASK1): task1.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

$(TASK2): task2.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(PTHREAD_FLAGS) $(LDLIBS)

$(TASK3): task3.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@ $(OPENMP_FLAGS) $(LDLIBS)

clean:
	$(RM) $(TASK1) $(TASK2) $(TASK3)
