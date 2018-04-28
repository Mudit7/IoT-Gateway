CC=arm-linux-gnueabihf-gcc

all:
	${CC} -fPIC -c main.c  
	gcc -c subscriber.c -I /usr/include/mysql/
clean:
	rm main.o
	rm subscriber.o
