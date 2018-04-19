CC=arm-linux-gnueabihf-gcc

all:bin/main 

bin/main:	
	make -C src
	${CC} -static src/main.o -I inc/ -L lib -lgateway -lkmlcd -lpthread -o bin/main
clean:
	rm bin/main
	make -C src clean

install:
	sudo cp bin/main /usr/bin

library:
	make -C lib 
	ar rcs lib/libgateway.a lib/gateway_lib.o 
	ar rcs lib/libkmlcd.a lib/km_lcd.o 
distclean:
	make -C lib clean
	make -C src clean	
	rm bin/main
copy:
	scp bin/main km@192.168.1.12:~/gateway
