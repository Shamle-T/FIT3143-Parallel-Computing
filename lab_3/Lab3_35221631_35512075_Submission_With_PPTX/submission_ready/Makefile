CC = mpicc
CFLAGS = -std=c11 -O2 -g -fopenmp -Wall -Wextra -Wpedantic -Werror
LDLIBS = -lm

.PHONY: all check
all: wsn

wsn: src/wsn.c
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

check: wsn
	python3 scripts/validate.py
