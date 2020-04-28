#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_NUMBER 3				// 线程数
#define REPEAT_NUMBER 2				// 每个线程任务数
#define DELAY_TIME_LEVELS 10.0		// 小任务之间地最大时间间隔


void *thrd_func(void *arg) {
	int thrd_num = (int) arg;
	int delay_time = 0;
	int count = 0;

	printf("Thread %d is starting\n", thrd_num);

	// 每个线程的任务
	for (count = 0; count < REPEAT_NUMBER; count++) {
		delay_time = (int)(rand() * DELAY_TIME_LEVELS/(RAND_MAX)) + 1;
		sleep(delay_time);
		printf("\t Thread %d: job %d delay = %d\n", thrd_num, count, delay_time);
	}

	printf("Thread %d finished\n", thrd_num);
	pthread_exit(NULL);
}

void *second_func(void *arg) {
	printf("This thread end...\n");
}

int main(void) {
	pthread_t thread[THREAD_NUMBER];	// 3
	int no = 0, res;
	void *thrd_ret;
	srand(time(NULL));
	for (no = 0; no < THREAD_NUMBER; no++) {
		// 创建多线程
		res = pthread_create(&thread[no], NULL, thrd_func, (void*)no);
		if (res != 0) {
			printf("Create thread %d failed\n", no);
			exit(res);
		}
	}

	printf("Create threads success\n Waiting for threads to finish...\n");
	for (no = 0; no < THREAD_NUMBER; no++) {
		// 等待线程结束
		res = pthread_join(thread[no], &thrd_ret);
		if (!res) {
			printf("Thread %d joined\n", no);
		} else {
			printf("Thread %d join failed\n", no);
		}
	}

	printf("Process Sleep...\n");
	sleep(3);

	// for (no = 0; no < THREAD_NUMBER; no++) {
	// 	// 创建多线程
	// 	res = pthread_create(&thread[no], NULL, second_func, (void*)no);
	// 	if (res != 0) {
	// 		printf("Create thread %d failed\n", no);
	// 		exit(res);
	// 	}
	// }

	// printf("Create threads success\n Waiting for threads to finish...\n");
	// for (no = 0; no < THREAD_NUMBER; no++) {
	// 	// 等待线程结束
	// 	res = pthread_join(thread[no], &thrd_ret);
	// 	if (!res) {
	// 		printf("Thread %d joined\n", no);
	// 	} else {
	// 		printf("Thread %d join failed\n", no);
	// 	}
	// }

	return 0;
}