/****************************************************************************************************************************************/
/*				File Name : Multithreading - Data sharing between multiple threads					*/
/*				Author : Sarthak Jain											*/
/*				Code written for Adanced Embedded Software Development under Prof. Rick Heidebrecht			*/
/*				Dated 2/28/2019												*/
/* Code description : A main thread with two child threads is created. All three threads access a common log file and write some of 	*/
/* their actions to it. Child 1 sorts and counts the alphabets of a text file, while Child 2 reports processor utilization every 100 	*/
/* milliseconds. Signal handlers and pthread timers are also used.								 	*/
/* Reference is given to: https://www.geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization/ ; GeeksforGeeks for mutex lock	*/
/* Reference is given to: http://devarea.com/linux-handling-signals-in-a-multithreaded-application/#.XHjC_YVlBwd ; DevArea for Signals 	*/
/* and : POSIX timer demo code, based on https://riptutorial.com/posix/example/16306/posix-timer-with-sigev-thread-notification 	*/
 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <syslog.h>
#include <ctype.h>

#define FILE_GDB ("gdb.txt")

typedef struct
{
	FILE* file_ptr;
	char* file_name;
} threadParams;

uint32_t test_char[26] = {0};
uint8_t sigflag = 0;
int sig_num;

pthread_t thread_1, thread_2;
pthread_attr_t att_1,att_2;
pthread_mutex_t lock;

struct timespec TimeSpec;

static timer_t timerid;
struct itimerspec trigger;
struct sigevent sev;

void mask_sig(void)
{
	sigset_t mask;
	sigemptyset(&mask); 
        sigaddset(&mask, SIGUSR1); 
                
        pthread_sigmask(SIG_BLOCK, &mask, NULL);      
}

void handler(int signo, siginfo_t *info, void *extra) 
{
	sigflag = 1;
	sig_num = signo;
}

void set_sig_handler(void)
{
        struct sigaction action;

        action.sa_flags = SA_SIGINFO; 
        action.sa_sigaction = handler;
 
        if (sigaction(SIGUSR1, &action, NULL) == -1) 
	{ 
		perror("sigusr: sigaction");
		_exit(1);
        } 

	if (sigaction(SIGUSR2, &action, NULL) == -1) 
	{ 
		perror("sigusr: sigaction");
		_exit(1);
	} 
}

void timer_handler(union sigval sv) 
{
	threadParams* timerArg = (threadParams* )sv.sival_ptr;
	char* cpu_util = malloc(127);

	if(pthread_mutex_lock(&lock) != 0)
	{
		perror("\nMutex lock failed\n");
		exit(1);
	}
		
	FILE* p = popen("cat /proc/stat | grep 'cpu'", "r");
	if(p == NULL)
	{
		printf("\nPipe open failed\n");
	}
	(*timerArg).file_ptr = fopen((*timerArg).file_name, "a");
	clock_gettime(CLOCK_REALTIME, &TimeSpec);
	fprintf((*timerArg).file_ptr, "\nTimer handler called at %lu secs, %lu msecs, %lu nsecs\n", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec);	
	while(fgets(cpu_util, sizeof(cpu_util) - 1, p) != NULL)
	{
		fprintf((*timerArg).file_ptr, "%s", cpu_util);
	}	
	fclose((*timerArg).file_ptr);
	pclose(p);
	pthread_mutex_unlock(&lock);
}

void *words(void* threadp)
{
	mask_sig();
	clock_gettime(CLOCK_REALTIME, &TimeSpec);
	char file_char;
	threadParams* threadArg = (threadParams *)threadp;

	if(pthread_mutex_lock(&lock) != 0)
	{
		perror("\nMutex lock failed\n");
		exit(1);
	}

	(*threadArg).file_ptr = fopen(FILE_GDB, "r");
	if((*threadArg).file_ptr == NULL)
	{
		perror("\nFile open failed\n");
		exit(1);
	}
	while(!feof((*threadArg).file_ptr))
	{
		file_char = fgetc((*threadArg).file_ptr);
		file_char = tolower(file_char);
		if(((uint32_t)file_char >= 97) && ((uint32_t)file_char <= 122))
		{
			test_char[(uint32_t)file_char - 97]++;
		}
	}

	(*threadArg).file_ptr = fopen((*threadArg).file_name, "a");
	if((*threadArg).file_ptr == NULL)
	{
		perror("\nFile open failed\n");
		exit(1);
	}
	fprintf((*threadArg).file_ptr, "\nChild thread 1 sector");
	fprintf((*threadArg).file_ptr, "\nChild thread 1 entry instant is at %lu secs, %lu msecs, %lu nsecs", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec);
	fprintf((*threadArg).file_ptr, "\nChild Thread 1 POSIX thread ID %lu and Linux Thread ID %lu", pthread_self(), syscall(SYS_gettid));
	for(int i=0; i<26;i++)
	{
		if(test_char[i] < 100)
		{
			fprintf((*threadArg).file_ptr, "\nCharacter %c has count %d", i+97, test_char[i]);
		}	
	}
	clock_gettime(CLOCK_REALTIME, &TimeSpec);
	fprintf((*threadArg).file_ptr, "\nChild thread 1 exit instant is at %lu secs, %lu msecs, %lu nsecs\n", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec);	
	fclose((*threadArg).file_ptr);
	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);
}

void *util(void* threadp)
{
	clock_gettime(CLOCK_REALTIME, &TimeSpec);
	threadParams* threadArg = (threadParams *)threadp;

	memset(&sev, 0, sizeof(struct sigevent));
	memset(&trigger, 0, sizeof(struct itimerspec));	

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = &timer_handler;
	sev.sigev_value.sival_ptr = threadArg;

	timer_create(CLOCK_REALTIME, &sev, &timerid);
	trigger.it_value.tv_nsec = 100000000;
	trigger.it_interval.tv_nsec = 100000000;
	timer_settime(timerid, 0, &trigger, NULL);

	if(pthread_mutex_lock(&lock) != 0)
	{
		perror("\nMutex lock failed\n");
		exit(1);
	}
	(*threadArg).file_ptr = fopen((*threadArg).file_name, "a");
	if((*threadArg).file_ptr == NULL)
	{
		perror("\nFile open failed\n");
		exit(1);
	}
	fprintf((*threadArg).file_ptr, "\nChild thread 2 sector");
	fprintf((*threadArg).file_ptr, "\nChild thread 2 entry instant is at %lu secs, %lu msecs, %lu nsecs", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec);
	fprintf((*threadArg).file_ptr, "\nChild Thread 2 POSIX thread ID %lu and Linux Thread ID %lu", pthread_self(), syscall(SYS_gettid));
	fclose((*threadArg).file_ptr);
	pthread_mutex_unlock(&lock);

	while(!sigflag);

	if(sigflag == 1)
	{
		sigflag = 0;
		if(pthread_mutex_lock(&lock) != 0)
		{
			perror("\nMutex lock failed\n");
			exit(1);
		}
		(*threadArg).file_ptr = fopen((*threadArg).file_name, "a");
		if((*threadArg).file_ptr == NULL)
		{
			perror("\nFile open failed\n");
			exit(1);
		}
		clock_gettime(CLOCK_REALTIME, &TimeSpec);
		if(sig_num == 10)
		{
			sig_num = 1;
			fprintf((*threadArg).file_ptr, "\nSignal USR%d received.\n", sig_num);
			fprintf((*threadArg).file_ptr, "\nChild thread 2 exit instant is at %lu secs, %lu msecs, %lu nsecs\n", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec);	
		}
		else if(sig_num == 12)
		{
			sig_num = 2;
			fprintf((*threadArg).file_ptr, "\nSignal USR%d received.\n", sig_num);
			fprintf((*threadArg).file_ptr, "\nChild thread 1 exit instant is at %lu secs, %lu msecs, %lu nsecs\n", TimeSpec.tv_sec, TimeSpec.tv_nsec/1000000, TimeSpec.tv_nsec);	
		}
		timer_delete(timerid);
		fclose((*threadArg).file_ptr);
		pthread_mutex_unlock(&lock);
		pthread_exit(NULL);
	}
}

int main (int argc, char *argv[])
{
	struct timespec MainTimeSpec;
	clock_gettime(CLOCK_REALTIME, &MainTimeSpec);
	int rc;
	threadParams threadParameter;
	threadParameter.file_name = argv[1];
	set_sig_handler();

	if(pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("\nMutex initialization failed\n");
	}

	if(pthread_mutex_lock(&lock) != 0)
	{
		perror("\nMutex lock failed\n");
		exit(1);
	}
	threadParameter.file_ptr = fopen(threadParameter.file_name, "a");
	if(threadParameter.file_ptr == NULL)
	{
		perror("\nFile open failed\n");
		exit(1);
	}
	fprintf(threadParameter.file_ptr, "\nMain thread sector");
	fprintf(threadParameter.file_ptr, "\nMain thread entry instant is at %lu secs, %lu msecs, %lu nsecs", MainTimeSpec.tv_sec, MainTimeSpec.tv_nsec/1000000, MainTimeSpec.tv_nsec);
	fprintf(threadParameter.file_ptr, "\nMain Thread POSIX thread ID %lu and Linux Thread ID %lu\n", pthread_self(), syscall(SYS_gettid));
	fclose(threadParameter.file_ptr);
	pthread_mutex_unlock(&lock);

	rc=pthread_attr_init(&att_1);       	//thread_1 initialization
	rc=pthread_attr_init(&att_2);		//thread_2 initialization

	pthread_create(&thread_1,   // pointer to thread descriptor
			&att_1,     // use default attributes
			words, // thread function entry point
			(void *)&(threadParameter) // parameters to pass in
			);


	pthread_create(&thread_2,   // pointer to thread descriptor
			&att_2,     // use default attributes
			util, // thread function entry point
			(void *)&(threadParameter) // parameters to pass in
			);

	pthread_join(thread_1, NULL);
	pthread_join(thread_2, NULL);

	if(pthread_mutex_lock(&lock) != 0)
	{
		perror("\nMutex lock failed\n");
		exit(1);
	}
	threadParameter.file_ptr = fopen(threadParameter.file_name, "a");
	if(threadParameter.file_ptr == NULL)
	{
		perror("\nFile open failed\n");
		exit(1);
	}
	clock_gettime(CLOCK_REALTIME, &MainTimeSpec);
	fprintf(threadParameter.file_ptr, "\nMain thread exit instant is at %lu secs, %lu msecs, %lu nsecs", MainTimeSpec.tv_sec, MainTimeSpec.tv_nsec/1000000, MainTimeSpec.tv_nsec);
	fprintf(threadParameter.file_ptr, "\nChildren are dead. My time has come.\n");
	fclose(threadParameter.file_ptr);
	pthread_mutex_unlock(&lock);

	pthread_mutex_destroy(&lock);
}

