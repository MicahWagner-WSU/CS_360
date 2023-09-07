main: main.o lineNum.o
	cc -g -o $@ $^
main.o: main.c lineNum.h
	cc -g -c main.c
lineNum.o: lineNum.c 
	cc -g -c lineNum.c 


.Phony: clean

clean:
	rm main.o lineNum.o main