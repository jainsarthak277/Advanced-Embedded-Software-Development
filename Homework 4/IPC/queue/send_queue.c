/****************************************************************************************************************************************/
/*				File Name : Inter-Process Communication using Messaging Queues						*/
/*				Author : Sarthak Jain											*/
/*				Code written for Adanced Embedded Software Development under Prof. Rick Heidebrecht			*/
/*				Dated 2/28/2019												*/
/* Code description : Two way communicatin is being performed between two processes. Both processes send as well as receive data in the */
/* form of a structure of 10 strings and an ON/OFF command meant for an LED.							 	*/
/* Reference is given to: Messaging Queues Code provided by Prof. Rick Heidebrecht 							*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <fcntl.h> 
#include <mqueue.h> 
#include <signal.h> 

#define MAX_SIZE (80)

typedef struct
{
	char* messages;
	uint8_t command;
} msg_seq;

struct timespec TimeSpec;

void handler(int signo, siginfo_t *info, void *extra) 
{
	FILE* file_ptr = fopen("log.txt", "a");
	if(file_ptr == NULL)
	{
		perror("\nFile open failed\n");
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &TimeSpec);
	fprintf(file_ptr, "\nExit signal received at %lu secs, %lu msecs, %lu nsecs", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec);
	fclose(file_ptr);
	exit(1);
}

void set_sig_handler(void)
{
        struct sigaction action;

        action.sa_flags = SA_SIGINFO; 
        action.sa_sigaction = handler;
 
        if (sigaction(SIGINT, &action, NULL) == -1) 
	{ 
		perror("sigint: sigaction");
		_exit(1);
        } 
}

int main()
{
	set_sig_handler();
	msg_seq all;
	char* songs[10] = {"We Don't talk anymore", "Wolves",
			 "It Ain't Me", "Gone gone gone", 
			 "Castle on the hill", "Shape of you",
			 "Hall of Fame", "Counting Stars",
			 "Girls Like You", "Something Just Like this"};
	uint8_t control[2] = {0,1};
	uint8_t j,k,l; 
	all.messages = malloc(30);	
	useconds_t usec = 100000;

	mqd_t queue_fd;
	struct mq_attr queue_attr;
	char string[MAX_SIZE];
	uint8_t len;
	
	queue_attr.mq_flags = 0;
    	queue_attr.mq_maxmsg = 10;
    	queue_attr.mq_msgsize = 30;
    	queue_attr.mq_curmsgs = 0;
	
	queue_fd = mq_open("/my_queue", O_RDWR | O_CREAT, 0664, &queue_attr);
	if(queue_fd == -1)
	{
		perror("Error opening queue");
		exit(EXIT_FAILURE);
	}

	FILE* file_ptr = fopen("log.txt", "a");
	if(file_ptr == NULL)
	{
		perror("\nFile open failed\n");
		exit(1);
	}
	fprintf(file_ptr, "\nQueue Messaging Process 2");
	fprintf(file_ptr, "\nProcess ID %u", getpid());
	fprintf(file_ptr, "\nUsing messaging queues with file descriptor queue_fd");

	for(int i=0; i<10; i++)
	{
		j = (rand()%10);
		k = (rand()%2);
		if(k == 0)
		{
			strcpy(all.messages, (rand()%2)?"TURN LED ON":"TURN LED OFF");
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
			len = mq_send(queue_fd, all.messages, 30, 0);	
			if (len < 0)
			{
				printf("\nSend failed\n");
			}	
		}
		else 
		{
			strcpy(all.messages, songs[j]);
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
			len = mq_send(queue_fd, all.messages, 30, 0);
			if (len < 0)
			{
				printf("\nSend failed\n");
			}
		}
		usleep(usec);
	}

	for(int i=0; i<10; i++)
	{
		len = mq_receive(queue_fd, string, 30, NULL);		
		if (len < 0)
		{
			printf("\nReceiving Read failed\n");
		}
		string[len] = 0;		//make sure it's a proper string
		clock_gettime(CLOCK_REALTIME, &TimeSpec);
		fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Received: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, string);		
		fprintf(file_ptr, "\nMain process received: %d bytes: %s", len, string);
		printf("Received %s\n", string);
	}
	fclose(file_ptr);
}
