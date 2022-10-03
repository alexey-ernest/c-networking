#include <stdio.h>
#include <time.h>

int main(int argc, char const *argv[])
{
	time_t timer;
	time(&timer);

	printf("Local time is: %s\n", ctime(&timer));

	return 0;
}