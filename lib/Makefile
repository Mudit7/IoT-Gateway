CC=arm-linux-gnueabihf-gcc

all:
	${CC} -c gateway_lib.c -fPIC -o gateway_lib.o 
	${CC} -c km_lcd.c -fPIC -o km_lcd.o 
clean:
	rm libkmlcd.so libgateway.so
	rm km_lcd.o
	rm gateway_lib.o

