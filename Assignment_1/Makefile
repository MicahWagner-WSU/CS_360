pairsofwords: main.o parse_utils.o
	cc -o $@ $^
main.o: main.c main.h
	cc -c main.c
parse_utils.o: parse_utils.c parse_utils.o
	cc -c parse_utils.c
