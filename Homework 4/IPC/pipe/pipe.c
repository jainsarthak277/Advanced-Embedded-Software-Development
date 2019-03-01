/****************************************************************************************************************************************/
/*				File Name : Inter-Process Communication using Pipes					*/
/*				Author : Sarthak Jain											*/
/*				Code written for Adanced Embedded Software Development under Prof. Rick Heidebrecht			*/
/*				Dated 2/28/2019												*/
/* Code description : Two way communicatin is being performed between two processes. Both processes send as well as receive data in the */
/* form of a structure of 10 strings and an ON/OFF command meant for an LED.							 	*/
/* Reference is given to: Pipe Code provided by Prof. Rick Heidebrecht 									*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <wait.h>

#define MAX_SIZE (80)

typedef struct
{
	char* messages;
	uint8_t command;
} msg_seq;

struct timespec TimeSpec;


int main()
{

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

	int pipe_1[2], pipe_2[2];

	pid_t mainpid, childpid;
	pthread_mutex_t lock;
	if(pthread_mutex_init(&lock, NULL))
	{
		perror("ERROR: pthread_mutex_init(), mutex not initialized");
		exit(EXIT_FAILURE);
	}

	pipe(pipe_1);
	pipe(pipe_2);

	mainpid = fork();
	if(mainpid == -1)
	{
		perror("Fork failed to create child\n");
		return(1);
	}

	if(mainpid > 0)
	{
		close(pipe_1[0]);
		close(pipe_2[1]);

		char string[MAX_SIZE];
		uint8_t len;

		pthread_mutex_lock(&lock);
		FILE* file_ptr = fopen("log.txt", "a");
		fprintf(file_ptr, "\nMain process");
		fprintf(file_ptr, "\nProcess ID %u\n", getpid());
		//srand(time(0));
		for(int i=0; i<10; i++)
		{
			j = (rand()%10);
			k = (rand()%2);
			if(k == 0)
			{
				strcpy(all.messages, (rand()%2)?"TURN LED ON":"TURN LED OFF");
				clock_gettime(CLOCK_REALTIME, &TimeSpec);
				fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
				write(pipe_1[1], all.messages, 3*sizeof(all.messages));		
			}
			else 
			{
				strcpy(all.messages, songs[j]);
				clock_gettime(CLOCK_REALTIME, &TimeSpec);
				fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
				write(pipe_1[1], all.messages, 3*sizeof(all.messages));
			}
			usleep(usec);
		}

		for(int i=0; i<10; i++)
		{
			len = read(pipe_2[0], string, MAX_SIZE);		
			if (len < 0)
			{
				printf("\nReceiving Read failed\n");
			}
			string[len] = 0;		//make sure it's a proper string
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Received: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, string);		
			fprintf(file_ptr, "\nMain process received: %d bytes: %s", len, string);
		}
		fprintf(file_ptr, "\n");
		fclose(file_ptr);	
		pthread_mutex_unlock(&lock);
	}
	else if (mainpid == 0)
	{
		close(pipe_1[1]);
		close(pipe_2[0]);

		char string[MAX_SIZE];
		uint8_t len;

		pthread_mutex_lock(&lock);
		FILE* file_ptr = fopen("log.txt", "a");
		fprintf(file_ptr, "\nChild process");
		fprintf(file_ptr, "\nProcess ID %u", getpid());
		srand(time(0));
		//return(0);

		for(int i=0; i<10; i++)
		{
			len = read(pipe_1[0], string, MAX_SIZE);		
			if (len < 0)
			{
				printf("\nReceiving Read failed\n");
			}
			string[len] = 0;		//make sure it's a proper string
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Received: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, string);	
			fprintf(file_ptr, "\nChild process received: %d bytes: %s", len, string);
		}

		for(int i=0; i<10; i++)
		{
			j = (rand()%10);
			k = (rand()%2);
			if(k == 0)
			{
				strcpy(all.messages, (rand()%2)?"TURN LED ON":"TURN LED OFF");
				clock_gettime(CLOCK_REALTIME, &TimeSpec);
				fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
				write(pipe_2[1], all.messages, 3*sizeof(all.messages));		
			}
			else 
			{
				strcpy(all.messages, songs[j]);
				clock_gettime(CLOCK_REALTIME, &TimeSpec);
				fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
				write(pipe_2[1], all.messages, 3*sizeof(all.messages));
			}
			usleep(usec);
		}

		fprintf(file_ptr, "\n");
		fclose(file_ptr);	
		pthread_mutex_unlock(&lock);

	}

	pthread_mutex_destroy(&lock);	
	return (0);
}
