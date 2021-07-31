#pragma once

#include <stdbool.h>

void *timer_thread_main(void *data);
bool tryLockPlug(int plug);
void unlockPlug(int plug);
void togglePlug(int plug);
bool getPlugState(int plug);
void setPlugState(int plug, bool state);
