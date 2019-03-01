/****************************************************************************************************************************************/
/*				File Name : Inter-Process Communication using Sockets, Client process					*/
/*				Author : Sarthak Jain											*/
/*				Code written for Adanced Embedded Software Development under Prof. Rick Heidebrecht			*/
/*				Dated 2/28/2019												*/
/* Code description : Two way communicatin is being performed between two processes. Both processes send as well as receive data in the */
/* form of a structure of 10 strings and an ON/OFF command meant for an LED.							 	*/
/* Reference is given to: http://www.it.uom.gr/teaching/distrubutedSite/dsIdaLiu/labs/lab2_1/sockets.html "Inter-Process Communication 	*/
/* with Sockets" for creation of sockets. 												*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>

#define SERV_TCP_PORT 8000 /* server's port number */
#define MAX_SIZE 80

typedef struct
{
	char* messages;
	uint8_t command;
} msg_seq;

struct timespec TimeSpec;

int main(int argc, char* argv[])
{
	msg_seq all;
	char* songs[10] = {"We Don't talk anymore", "Wolves",
			 "It Ain't Me", "Gone gone gone", 
			 "Castle on the hill", "Shape of you",
			 "Hall of Fame", "Counting Stars",
			 "Girls Like You", "Something Just Like this"};
	uint8_t control[2] = {0,1};
	uint8_t j,k,l; 
	srand(time(0));
	all.messages = malloc(30);
	int len;
	useconds_t usec = 10000;

	struct sockaddr_in serv_addr;//, cli_addr;
	char *serv_host = "localhost";
	struct hostent *host_ptr;
	int buff_size = 0;
	int client_fd, newclient_fd, clilen, port;
	char string[MAX_SIZE];

	port = SERV_TCP_PORT;

	if((host_ptr = gethostbyname(serv_host)) == NULL) 
	{
		perror("gethostbyname error");
		exit(1);
	}
  
  	if(host_ptr->h_addrtype !=  AF_INET) 
	{
		perror("unknown address type");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ((struct in_addr *)host_ptr->h_addr_list[0])->s_addr;
	serv_addr.sin_port = htons(port);

	if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("CAN'T OPEN SOCKET");
		exit(EXIT_FAILURE);
	}
    
	if(connect(client_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 		// connect to the server
	{
		perror("can't connect to server");
		exit(1);
	}

	FILE* file_ptr = fopen("log.txt", "a");
	fprintf(file_ptr, "\nClient Process");
	fprintf(file_ptr, "\nClient Process ID %u", getpid());
	fprintf(file_ptr, "\nUsing sockets with file descriptor client_fd");

	for(int i=0; i<10; i++)
	{
		j = (rand()%10);
		k = (rand()%2);
		if(k == 0)
		{
			strcpy(all.messages, (rand()%2)?"TURN LED ON":"TURN LED OFF");
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
			write(client_fd, all.messages, 3*sizeof(all.messages));		// write a message to the server
		}
		else 
		{
			strcpy(all.messages, songs[j]);
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
			write(client_fd, all.messages, 3*sizeof(all.messages));
		}
		usleep(usec);
	}

	for(int i=0; i<10; i++)
	{
		len = read(client_fd, string, MAX_SIZE);		// read a message from the client  
		if (len < 0)
		{
			printf("\nReceiving Read failed\n");
		}
		string[len] = 0;		//make sure it's a proper string
		clock_gettime(CLOCK_REALTIME, &TimeSpec);
		fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Received: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, string);
		printf("%s\n", string);
	}

	close(client_fd);
}

