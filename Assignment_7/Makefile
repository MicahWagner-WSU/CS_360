main: main.o assignment7.o 
	cc -o $@ $^
main.o: main.c assignment7.h
	cc -c main.c
assignment7.o: assignment7.c assignment7.h
	cc -c assignment7.c

.Phony: clean

clean:
	rm main.o assignment7.o main


