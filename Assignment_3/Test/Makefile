main: main.o assignment3.o
	cc -g -o $@ $^
main.o: main.c assignment3.h
	cc -g -c main.c
assignment3.o: assignment3.c 
	cc -g -c assignment3.c 


.Phony: clean

clean:
	rm main.o assignment3.o main