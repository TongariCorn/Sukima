.PHONY: all

all:
	gcc -Wall core.c env.c eval.c gc.c main.c obj.c printError.c parser.c sukima.c
