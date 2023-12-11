myftpserve myftp: myftp.o myftpserve.o myftp.h
	cc -o myftpserve myftpserve.o
	cc -o myftp myftp.o
myftp.o: myftp.c myftp.h
	cc -c myftp.c
myftpserve.o: myftpserve.c myftp.h
	cc -c myftpserve.c

.Phony: clean

clean:
	rm myftp.o myftpserve.o myftpserve myftp