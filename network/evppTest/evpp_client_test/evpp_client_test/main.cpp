#include <cstdio>

#include "test_core.h"

int main()
{
	test_core core;

	core.Start();


	while (1)
	{
		sleep(10);
	}

    return 0;
}