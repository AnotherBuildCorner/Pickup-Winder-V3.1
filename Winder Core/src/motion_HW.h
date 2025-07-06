// motion.h
#pragma once
#include "global_vars.h"
#include <Arduino.h>
#include <TMCStepper.h>

void initMotionPins();
void initTMC();
bool homeCycle(int rate, bool dir, float backoffDistance = 10.0f, unsigned long timer_ms = 0, int refresh_Time = 100);
std::vector<int> precomputeMultipliers(int turns, int gauge, float width_mm, float overwind_percent, float center_space_mm, float faceplate_thickness, float edge_error = 0.0f);

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
  int _currentStepRate = 0; // Current step rate in steps per second
};

class TraverseStepper {
public:
  TraverseStepper(gpio_num_t stepPin, gpio_num_t dirPin, int ledcChannel, gpio_num_t enablePin,gpio_num_t readbackPin = GPIO_NUM_NC);
  void begin();
  void setEnabled(bool run);
  void setRate(float stepsPerSecond);
  void setLimits(int currentpos);
  void setPosition(int pos);
  int getPosition() const;
  void setZero();
  void setDirection(bool clockwise);
  void enableHoming(gpio_num_t pin, bool activeLow = true);
  void checkHome();
  bool isHomed();
  int distanceToSteps(float distance);
  bool homeProcess(int rate, bool dir); // Back off after homing
  void compute_backoff(float distance);
  void setRPM(int RPM);
  void setTraverseRate(int spindleStepRate, float wireGauge);
  void controlPosition(); // Control position based on step count
  int getLayerCount(); 
  void jogDistance(int speed, float distance, bool direction, bool stopAfter = true);
  void setMultiplier(); // Jog the stepper a certain distance at a given speed
  float computePosition();
  float getMultiplier();
  void loadCompParameters(const WinderPreset& preset);
  int computeLayers();
  int computeLength();
  int computeLiveDCR(int currentLayer, int current_steps);
  void UIRunonce();

    // Implement acceleration ramp logic here
    // This is a placeholder for the actual implementation
  
private:
  gpio_num_t _stepPin, _dirPin, _enablePin, _readbackPin ,_homePin;
  std::string _gauge_type;
  int _pos, _posMin, _posMax, _holdPos=0;
  bool _homeActiveLow, _homed, _dirState;
  int _ledcChannel;
  int _layerCount = 1;
  float _multiplier = 1.0f, _backoff = 0.0f;
  unsigned long _stepcount;
  int _turns, _layers, _totalLength;
  float _width, _length, _height, _internal_error, _gauge, _DCR, _offset;
  std::vector<float> _pattern;
  std::vector<int> _turnsPerLayer, _lengthPerLayer;
};

extern AxisStepper spindle;
extern TraverseStepper traverse;