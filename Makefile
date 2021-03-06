CC=arm-linux-gnueabihf-gcc

all:bin/main 

bin/main:src/main.o
	${CC} src/main.o -I inc/ -L lib -lgateway -lkmlcd -lmosquitto -lpthread -o bin/main
	gcc src/subscriber.o -I /usr/include/mysql/ -lmosquitto -lmysqlclient -o bin/sub

src/main.o: 
	make -C src
clean:
	rm bin/main bin/sub
	make -C src clean
install:
	sudo cp bin/main /usr/bin

library:
	make -C lib 
#	ar rcs lib/libgateway.a lib/gateway_lib.o 
#	ar rcs lib/libkmlcd.a lib/km_lcd.o 
	gcc -shared -o lib/libkmlcd.so lib/km_lcd.o 
	gcc -shared -o lib/libgateway.so lib/gateway_lib.o
distclean:
	make -C lib clean
	make -C src clean	
	rm bin/main bin/sub
copy:
	scp bin/main km@192.168.1.12:~/gateway
