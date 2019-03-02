/****************************************************************************************************************************************/
/*				File Name : Inter-Process Communication using Shared Memory, Producer process				*/
/*				Author : Sarthak Jain											*/
/*				Code written for Adanced Embedded Software Development under Prof. Rick Heidebrecht			*/
/*				Dated 2/28/2019												*/
/* Code description : Two way communicatin is being performed between two processes. Both processes send as well as receive data in the */
/* form of a structure of 10 strings and an ON/OFF command meant for an LED.								*/
/* Reference given to shared memory codes provided by Prof. Rick Heidebrecht								*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#define MAX_SIZE 80

typedef struct
{
	char* messages;
	uint8_t command;
} msg_seq;

struct timespec TimeSpec;

sem_t* sem1;
sem_t* sem2;

const char* memory = "OS\n";

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
	sem_close(sem1);
	sem_close(sem2);
	shm_unlink(memory);
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

int main(int argc, char* argv[])
{
	set_sig_handler();
	msg_seq all, string;
	char* songs[10] = {"We Don't talk anymore", "Wolves",
			 "It Ain't Me", "Gone gone gone", 
			 "Castle on the hill", "Shape of you",
			 "Hall of Fame", "Counting Stars",
			 "Girls Like You", "Something Just Like this"};
	uint8_t control[2] = {0,1};
	uint8_t j,k,l; 
	srand(time(0));
	all.messages = malloc(30);
	string.messages = malloc(MAX_SIZE);
	int len;
	useconds_t usec = 10000;
	int buff_size = 0, check;

	const int size = 1024;
	const char* sem1_name = "sem1";
	const char* sem2_name = "sem2";
	
	int shm_fd ;
	void* ptr;

	const char* message0 = "Hello";
	const char* message1 = "World\n";	

	if((shm_fd = shm_open(memory, O_CREAT | O_RDWR, 0666)) ==  -1)
	{
		perror("Error opening shared memory descriptor");
		exit(EXIT_FAILURE);
	}

	sem1 = sem_open(sem1_name, O_CREAT, 0664, 0);
	if(sem1 == SEM_FAILED)
	{
		perror("Error creating named semaphore");
		exit(EXIT_FAILURE);
	}

	sem2 = sem_open(sem2_name, O_CREAT, 0664, 0);
	if(sem2 == SEM_FAILED)
	{
		perror("Error creating named semaphore");
		exit(EXIT_FAILURE);
	}	
	check = ftruncate(shm_fd, size);
	if(check == -1)
	{
		perror("Error truncating file descriptor");
		exit(EXIT_FAILURE);
	}

	if((ptr = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
	{
		perror("Error mapping memory");
		exit(EXIT_FAILURE);
	}

	FILE* file_ptr = fopen("log.txt", "a");
	fprintf(file_ptr, "\nProducer Process");
	fprintf(file_ptr, "\nProcess ID %u", getpid());
	fprintf(file_ptr, "\nUsing shared memory with file descriptor shm_fd");

	for(int i=0; i<10; i++)
	{
		j = (rand()%10);
		k = (rand()%2);
		if(k == 0)
		{
			strcpy(all.messages, (rand()%2)?"TURN LED ON":"TURN LED OFF");
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
			memcpy(ptr, all.messages, MAX_SIZE);
			ptr += MAX_SIZE; 
		}
		else 
		{
			strcpy(all.messages, songs[j]);
			clock_gettime(CLOCK_REALTIME, &TimeSpec);
			fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Sending: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, all.messages);
			memcpy(ptr, all.messages, MAX_SIZE);
			ptr += MAX_SIZE; 		
		}
		usleep(usec);
	}
	fclose(file_ptr);

	ptr -= (10*MAX_SIZE);	
	sem_post(sem1);
	sem_wait(sem2);

	file_ptr = fopen("log.txt", "a");
	for(int i=0; i<10; i++)
	{
		memcpy(string.messages, ptr, MAX_SIZE);
		ptr += MAX_SIZE;  
		if (len < 0)
		{
			printf("\nReceiving Read failed\n");
		}
		string.messages[len] = 0;		//make sure it's a proper string
		clock_gettime(CLOCK_REALTIME, &TimeSpec);
		fprintf(file_ptr, "\n[Timestamp %lu secs, %lu msecs, %lu nsecs] Received: <%s>", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec, string.messages);
		fprintf(file_ptr, "\nProducer process received: %ld bytes: %s", strlen(string.messages), string.messages);
	}
	fclose(file_ptr);
//while(1);

	sem_close(sem1);
	sem_close(sem2);
	shm_unlink(memory);
}

