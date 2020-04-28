#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string>

int getrand(int num, int start)
{
	return rand()%num + start;
}

int num = 50;

// 参数 执行次数
int main(int argc, char* argv[])
{
	if (argc > 1) {
		num = std::stoi(argv[1]);
	}

	srand((unsigned)time(NULL));

	char *cmd[6];
	cmd[0] = (char*)"curl -i -X GET http://127.0.0.1:8888/";
	cmd[1] = (char*)"curl -i -X DELETE http://127.0.0.1:8888/";
	cmd[2] = (char*)"curl -i -X GET http://127.0.0.1:8888/TTT.html";
	cmd[3] = (char*)"curl -i -X POST --data 'Name=ZhaoKai&ID=113' http://127.0.0.1:8888/Post_show";
	cmd[4] = (char*)"curl -i -X POST --data 'Name=ZhaoKai&ID=113' http://127.0.0.1:8888/Post";
	cmd[5] = (char*)"curl -i -X POST --data 'Name=ZhaoKai&ID=113&&&' http://127.0.0.1:8888/Post_show";

	timeval start, end;

	gettimeofday(&start, 0);

	for (int i = 0; i < num; i++) {
		system(cmd[getrand(6, 0)]);
		// usleep(getrand(200000, 0));	// 0 - 0.2
	}

	gettimeofday(&end, 0);
	double timeuse  = 1000000*(end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
	printf("time = %f s\n", timeuse/1000000);
	return 0;
}