#include <stdatomic.h>
#include <stdbool.h>

#include "threadutils.h"

volatile static atomic_bool __appRunning = true;

bool appRunning()
{
	return __appRunning;
}

void stopThreads()
{
	__appRunning = false;
}
