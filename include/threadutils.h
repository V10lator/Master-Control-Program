#pragma once

#include <stdbool.h>

bool appRunning();
bool startThread(const char *name, bool realtime, void *(*function)(void *), void *data);
void stopThreads();
void dummyHandler(int signal);
