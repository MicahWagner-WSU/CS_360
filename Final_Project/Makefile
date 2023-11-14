mftpserve mftp: myftp.o myftpserve.o
	cc -o mftpserve myftpserve.o
	cc -o mftp myftp.o
mftp.o: myftp.c myftp.h
	cc -c myftp.c
mftpserve.o: myftpserve.c myftp.h
	cc -c myftpserve.c

.Phony: clean

clean:
	rm mftp.o mftpserve.o mftpserve mftp