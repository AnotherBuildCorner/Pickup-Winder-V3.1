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


class AxisStepper {
public:
  AxisStepper(gpio_num_t stepPin, gpio_num_t dirPin, gpio_num_t enablePin = GPIO_NUM_NC);
  void begin(bool clockwise = true);
  void setDirection(bool clockwise);
  void setEnabled(bool run);
  void setRate(float stepsPerSecond);
  void setAcceleration(float stepsPerSec2);
  void tick();
  int getStepCount() const;
    float getTurnCount() const;
    void resetStepCount();
private:
  gpio_num_t _stepPin, _dirPin, _enablePin;
  uint32_t _accum;
  float _targetRate, _currentRate, _accelPerTick;
  bool _stepState, _run, _dirState;
  static constexpr uint32_t SCALE = 100000;
  static constexpr uint32_t ISR_FREQ = 1000*1000/ISR_delay;
  uint32_t _stepCount = 0;
};

class TraverseStepper {
public:
  TraverseStepper(gpio_num_t stepPin, gpio_num_t dirPin, gpio_num_t enablePin);
  void begin();
  void setRate(float stepsPerSecond);
  void setLimits(int minPos, int maxPos);
  void tick();
  void setPosition(int pos);
  int getPosition() const;
  void setZero();
  void enableHoming(gpio_num_t pin, bool activeLow = true);
private:
  gpio_num_t _stepPin, _dirPin;
  uint32_t _accum, _increment;
  float _rate;
  bool _stepState, _dirState, _run;
  int _pos, _posMin, _posMax;
  gpio_num_t _homePin;
  bool _homeActiveLow;
  bool _homed;
  void checkHome();
  static constexpr uint32_t SCALE = 100000;
  static constexpr uint32_t ISR_FREQ = 1000*1000/ISR_delay;
};

// Externally defined global instances
extern AxisStepper spindle;
extern TraverseStepper traverse;