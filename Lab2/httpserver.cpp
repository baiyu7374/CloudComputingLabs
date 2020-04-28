#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <thread>
#include <string.h>

#include "ThreadPool.h"
#include "task.h"

using namespace std;

#define QUEUE 20

int POOL_SIZE = 8;
int SERVER_PORT = 8888;
string SERVER_IP = "127.0.0.1";

// 线程的任务
void thread_task(int client_fd) {
	task *ta = new task(client_fd);
	ta->StartDeal();
} 

// 指定 ip, port, 线程池大小
int main(int argc, char* argv[])
{
	for (int i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "--ip") == 0) {
			SERVER_IP = argv[i+1];
			i++;
		} else if (strcmp(argv[i], "--port") == 0) {
			SERVER_PORT = stoi(argv[i+1]);
			i++;
		} else if (strcmp(argv[i], "--number-thread") == 0) {
			POOL_SIZE = stoi(argv[i+1]);
			i++;
		}
	}

	cout << "Server ip: " << SERVER_IP << "  Server port: " << SERVER_PORT << "  ThreadPool Size: " << POOL_SIZE << endl;

	int server_fd, client_fd;
	struct sockaddr_in server_addr;

	// set server's sockaddr
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// create socket
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket create error\n");
		exit(1);
	}

	// bind
	if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		printf("bind server error\n");
		exit(1);
	}

	// listen
	if (listen(server_fd, QUEUE) < 0) {
		printf("listen error\n");
		exit(1);
	}


	// create & init Thread Pool
	ThreadPool pool(POOL_SIZE);
	pool.init();


	// accept client's connection.
	struct sockaddr_in client_addr;
	socklen_t length = sizeof(client_addr);

	while (1)
	{
		client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &length);

		// 向线程池添加任务.
		pool.submit(thread_task, client_fd);
	}
   
	return 0;
}