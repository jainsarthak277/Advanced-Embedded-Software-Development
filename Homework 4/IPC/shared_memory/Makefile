all: producer_xc consumer_xc

producer_xc: prod.c
	gcc prod.c -o producer_xc -lpthread -lrt -g 

consumer_xc: cons.c
	gcc cons.c -o consumer_xc -lpthread -lrt -g

clean:
	-rm -f producer_xc consumer_xc prod.o cons.o log.txt 

