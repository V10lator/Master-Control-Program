#pragma once

#include <stdbool.h>

void setupPlug(int plug);
bool tryLockPlug(int plug);
bool tryLockPlugStrong(int plug);
void unlockPlug(int plug);
bool isPlugLocked(int plug);
void togglePlug(int plug);
bool getPlugState(int plug);
void setPlugState(int plug, bool state);
