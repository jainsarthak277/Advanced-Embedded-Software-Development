all: receive_xc send_xc

receive_xc: receive_queue.o
	gcc -O0 -g -o receive_xc receive_queue.o -lrt

send_xc: send_queue.o
	gcc -O0 -g -o send_xc send_queue.o -lrt

clean:
	-rm -f send_xc receive_xc send_queue.o receive_queue.o log.txt

