#pragma once

#include <stdbool.h>

bool appRunning();
bool startThread(bool realtime, void *(*function)(void *), void *data);
void stopThreads();
void dummyHandler(int signal);
