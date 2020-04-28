#include <stdio.h>
#include <stdlib.h>


void *func1(void *arg) {
	int num = (int) arg;
	printf("num\n");
	return (void *)true;
}

int main()
{
	int x = 1;
	bool x = (bool)func1((void *)x);
	printf("%d \n", x);
}