#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <time.h>
#define SIZE 256

int main()
{
	int32_t* data_buffer = (int32_t*)malloc(SIZE*sizeof(int32_t));
	int32_t* syscall_output = (int32_t*)malloc(SIZE*sizeof(int32_t));;
	uint8_t syscall_result;
	time_t sec;

	clock_t start, stop;
	double diff;

	sec = time(NULL);
	sec = (int)(sec/288);
	srand(sec);
	
	if(data_buffer == NULL)
	{
		printf("\nInput buffer allocation failed\n");
	}
	if(syscall_output == NULL)
	{
		printf("\nOutput data buffer allocation failed\n");
	}

	printf("Original Array is :\n");
	for(int i =0; i<SIZE; i++)
	{
		data_buffer[i] = rand();
		printf("random number %d: %d\n", i, data_buffer[i]);
	}

	start = clock();
	syscall_result = syscall(399, data_buffer, SIZE, syscall_output);
	stop = clock() - start;
	
	printf("System call sys_sort returned %d\n", syscall_result);
	printf("Sorted Array is :\n");
	for (int i=0; i<SIZE; i++)
	{
		printf("sorted number %d: %d\n", i, syscall_output[i]);
	}

	diff = ((double)stop)/CLOCKS_PER_SEC;
	printf("Time taken for system call: %f\n", diff);
	free(data_buffer);
	free(syscall_output);
	return 0;
}
