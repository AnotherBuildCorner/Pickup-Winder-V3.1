// motion.cpp
#include "motion.h"
#include "config.h"
// Default pin assignments


void initMotionPins() {
  pinMode(SPINDLE_STEP_PIN, OUTPUT);
  pinMode(SPINDLE_DIR_PIN, OUTPUT);
  pinMode(TRAVERSE_STEP_PIN, OUTPUT);
  pinMode(TRAVERSE_DIR_PIN, OUTPUT);
}

void rotateSpindleTurns(int turns, int stepsPerRev, int delayMicros, bool direction) {
  digitalWrite(SPINDLE_DIR_PIN, direction);
  int totalSteps = turns * stepsPerRev;
  for (int i = 0; i < totalSteps; i++) {
    digitalWrite(SPINDLE_STEP_PIN, HIGH);
    delayMicroseconds(delayMicros);
    digitalWrite(SPINDLE_STEP_PIN, LOW);
    delayMicroseconds(delayMicros);
  }
}

void jogTraverseSteps(int steps, int delayMicros, bool direction) {
  digitalWrite(TRAVERSE_DIR_PIN, direction);
  for (int i = 0; i < steps; i++) {
    digitalWrite(TRAVERSE_STEP_PIN, HIGH);
    delayMicroseconds(delayMicros);
    digitalWrite(TRAVERSE_STEP_PIN, LOW);
    delayMicroseconds(delayMicros);
  }
}
