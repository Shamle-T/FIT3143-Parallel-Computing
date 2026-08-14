CC = gcc
CFLAGS = -std=c11 -O3 -Wall -Wextra -pedantic

.PHONY: all clean

all: task1 task2 task3

task1: task1.c
	$(CC) $(CFLAGS) $< -o $@

task2: task2.c
	$(CC) $(CFLAGS) $< -o $@ -pthread

task3: task3.c
	$(CC) $(CFLAGS) $< -o $@ -fopenmp

clean:
	$(RM) task1 task2 task3 task1.exe task2.exe task3.exe
