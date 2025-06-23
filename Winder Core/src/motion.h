// motion.h
#pragma once
#include "global_vars.h"
#include <Arduino.h>
#include <TMCStepper.h>

// Allow pin remapping here if needed

void initMotionPins();
void rotateSpindleTurns(bool direction);
void jogTraverseSteps(int steps, int delayMicros, bool direction);
void initTMC();
