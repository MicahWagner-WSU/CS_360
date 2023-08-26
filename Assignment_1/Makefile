pairsofwords: main.o parse_utils.o hash.o
	cc -o $@ $^
main.o: main.c 
	cc -c main.c
parse_utils.o: parse_utils.c parse_utils.h
	cc -c parse_utils.c
hash.o: hash.c hash.h
	cc -c hash.c

.Phony: clean

clean:
	rm main.o parse_utils.o hash.o


