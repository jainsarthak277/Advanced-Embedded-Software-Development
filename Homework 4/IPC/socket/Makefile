all: servsocket_xc clisocket_xc

servsocket_xc: servsocket.o
	gcc -O0 -g -o servsocket_xc servsocket.o 

clisocket_xc: clisocket.o
	gcc -O0 -g -o clisocket_xc clisocket.o

clean:
	-rm -f servsocket_xc clisocket_xc servsocket.o clisocket.o log.txt

