// motion.h
#pragma once
#include "config.h"
#include <Arduino.h>

// Allow pin remapping here if needed

void initMotionPins();
void rotateSpindleTurns(int turns, int stepsPerRev, int delayMicros, bool direction);
void jogTraverseSteps(int steps, int delayMicros, bool direction);
