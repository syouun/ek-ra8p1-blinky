CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror -O2

all: run

fall_judge_test: fall_judge_test.c ../mtk3bsp2_ra8p1_ek/Application/fall_judge.c ../mtk3bsp2_ra8p1_ek/Application/fall_judge.h
	$(CC) $(CFLAGS) -o $@ fall_judge_test.c ../mtk3bsp2_ra8p1_ek/Application/fall_judge.c

run: fall_judge_test
	./fall_judge_test

clean:
	rm -f fall_judge_test

.PHONY: all run clean
