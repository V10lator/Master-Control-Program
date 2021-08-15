#include <stdbool.h>

#include "threadutils.h"

volatile static bool __appRunning = true;

bool appRunning()
{
	return __appRunning;
}

void stopThreads()
{
	__appRunning = false;
}

void dummyHandler(int signal) {};
