// motion.h
#pragma once
#include "global_vars.h"
#include <Arduino.h>
#include <TMCStepper.h>

void initMotionPins();
void initTMC();

class AxisStepper {
public:
  AxisStepper(gpio_num_t stepPin, gpio_num_t dirPin, int ledcChannel, gpio_num_t enablePin = GPIO_NUM_NC ,gpio_num_t readbackPin = GPIO_NUM_NC);
  void begin(bool clockwise = true);
  void setDirection(bool clockwise);
  void setEnabled(bool run);
  void setRate(float stepsPerSecond);
  int getStepCount() const;
  float getTurnCount() const;
  void resetStepCount();
  void rampAcceleration(int targetRPM, float rampRate = 10.0f, bool decelerate = false, unsigned long timer_ms = 0, int refresh_Time = 100);
  void getRate(unsigned long timer_ms, int refresh_Time = 200); // Read the speed pot value and update target RPM
private:
  gpio_num_t _stepPin, _dirPin, _enablePin, _readbackPin;
  float _targetRate;
  bool _dirState;
  uint32_t _stepCount;
  int _ledcChannel;
};

class TraverseStepper {
public:
  TraverseStepper(gpio_num_t stepPin, gpio_num_t dirPin, int ledcChannel, gpio_num_t enablePin);
  void begin();
  void setEnabled(bool run);
  void setRate(float stepsPerSecond);
  void setLimits(int minPos, int maxPos);
  void setPosition(int pos);
  int getPosition() const;
  void setZero();
  void setDirection(bool clockwise);
  void enableHoming(gpio_num_t pin, bool activeLow = true);
  void checkHome();
  bool isHomed();
  
    // Implement acceleration ramp logic here
    // This is a placeholder for the actual implementation
  
private:
  gpio_num_t _stepPin, _dirPin, _enablePin;
  int _pos, _posMin, _posMax;
  gpio_num_t _homePin;
  bool _homeActiveLow, _homed, _dirState;
  int _ledcChannel;
  
};

extern AxisStepper spindle;
extern TraverseStepper traverse;