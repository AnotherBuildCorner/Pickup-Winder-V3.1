// motion.cpp
#include "motion_HW.h"
#include <driver/ledc.h>

#define R_SENSE 0.11f
#define Spindle_ADDRESS 0b00
#define Traverse_ADDRESS 0b01
#define DUAL_SERIAL
#define SENSORLESS_HOME

#ifdef DUAL_SERIAL
HardwareSerial& TMCSerial = Serial1;
HardwareSerial& TMCSerial_traverse = Serial2;
TMC2209Stepper spindle_driver(&TMCSerial, R_SENSE, Spindle_ADDRESS);
TMC2209Stepper traverse_driver(&TMCSerial_traverse, R_SENSE, Traverse_ADDRESS);
#else
HardwareSerial& TMCSerial = Serial1;
TMC2209Stepper spindle_driver(&TMCSerial, R_SENSE, Spindle_ADDRESS);
TMC2209Stepper traverse_driver(&TMCSerial, R_SENSE, Traverse_ADDRESS);
#endif

#define SPINDLE_TOFF
#define SPINDLE_BLANK_TIME 24
#define SPINDLE_SPREADCYCLE true
#define SPINDLE_PWM_AUTOSCALE true
#define SPINDLE_PWM_FREQ 2
#define SPINDLE_I_AUTOSCALE false
#define SPINDLE_RMS_CURRENT 2000 // RMS current in mA

#define TRAVERSE_TOFF 4
#define TRAVERSE_BLANK_TIME 24
#define TRAVERSE_SPREADCYCLE true
#define TRAVERSE_PWM_AUTOSCALE true
#define TRAVERSE_PWM_FREQ 2
#define TRAVERSE_I_AUTOSCALE false
#define TRAVERSE_RMS_CURRENT 600 // RMS current in mA

#define TRAVERSE_STALLGUARD 100// 0-255

void IRAM_ATTR onSpindleStep() {
  spindleStepCount++;
}

AxisStepper::AxisStepper(gpio_num_t stepPin, gpio_num_t dirPin, int ledcChannel, gpio_num_t enablePin, gpio_num_t readbackPin)
  : _stepPin(stepPin), _dirPin(dirPin), _enablePin(enablePin), _targetRate(0),
    _dirState(true), _stepCount(0), _ledcChannel(ledcChannel), _readbackPin(readbackPin) {}

void AxisStepper::begin(bool clockwise) {
  pinMode(_dirPin, OUTPUT);
  setDirection(clockwise);

  if (_enablePin != GPIO_NUM_NC) {
    pinMode(_enablePin, OUTPUT);
    gpio_set_level(_enablePin, HIGH);
  }

  // LEDC setup
  ledcSetup(_ledcChannel, 5000, 2);  // 5 kHz default
  ledcAttachPin(_stepPin, _ledcChannel);
  

  // Count steps using interrupts
  pinMode(_readbackPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(SPINDLE_READBACK_PIN), onSpindleStep, RISING);

}

void AxisStepper::setDirection(bool clockwise) {
  _dirState = clockwise;
  gpio_set_level(_dirPin, _dirState);
}

void AxisStepper::setEnabled(bool run) {
  if (_enablePin != GPIO_NUM_NC) {
    gpio_set_level(_enablePin, run ? LOW : HIGH);
  }
}

void AxisStepper::setRate(float RPM) {
  if (RPM > maxrpm) {
    RPM = maxrpm; } // Cap at max RPM
  Current_Step_Rate = (RPM * steps_rev * SPINDLE_MICROSTEPS) / 60.0;
  ledcWriteTone(_ledcChannel, Current_Step_Rate);
}

void AxisStepper::rampAcceleration(float targetRPM, float ramp_rate, bool decelerate, unsigned long timer_ms ,int refresh_Time) {
  static unsigned long lastRampTime = 0;
  if(timer_ms - lastRampTime < refresh_Time) return; // Limit ramping to every 100ms
  lastRampTime = timer_ms;
  if (targetRPM > maxrpm) {targetRPM = maxrpm; } // Cap at max RPM
  if(Current_RPM != targetRPM){ // Prevent negative RPM
    if(decelerate)
      {Current_RPM -= (ramp_rate*refresh_Time/1000);}
    else
      {Current_RPM += (ramp_rate*refresh_Time/1000);}
  }
  if(Current_RPM < 0) {
    Current_RPM = 0; } // Prevent negative RPM

    setRate(Current_RPM); // Update step rate
  }


int AxisStepper::getStepCount() const {  // Read the global step count
  return spindleStepCount;
}

float AxisStepper::getTurnCount() const {
  return static_cast<float>(spindleStepCount) / ( steps_rev* SPINDLE_MICROSTEPS);
}

void AxisStepper::resetStepCount() {
  spindleStepCount = 0;
}

TraverseStepper::TraverseStepper(gpio_num_t stepPin, gpio_num_t dirPin, int ledcChannel, gpio_num_t enablePin)
  : _stepPin(stepPin), _dirPin(dirPin), _enablePin(enablePin), _pos(0),
    _posMin(0), _posMax(1000), _homePin(GPIO_NUM_NC), _homeActiveLow(true), _homed(false), _ledcChannel(ledcChannel) {}

void TraverseStepper::begin() {
  pinMode(_dirPin, OUTPUT);
  gpio_set_level(_dirPin, HIGH);

  if (_enablePin != GPIO_NUM_NC) {
    pinMode(_enablePin, OUTPUT);
    gpio_set_level(_enablePin, HIGH);
  }

  ledcSetup(_ledcChannel, 5000, 1);
  ledcAttachPin(_stepPin, _ledcChannel);
}

void TraverseStepper::setRate(float stepsPerSecond) {
  ledcWriteTone(_ledcChannel, stepsPerSecond);
}

void TraverseStepper::setEnabled(bool run) {
  if (_enablePin != GPIO_NUM_NC) {
    gpio_set_level(_enablePin, run ? LOW : HIGH);
  }
}

void TraverseStepper::setLimits(int minPos, int maxPos) {
  _posMin = minPos;
  _posMax = maxPos;
}

void TraverseStepper::setPosition(int pos) {
  _pos = pos;
}

int TraverseStepper::getPosition() const {
  return _pos;
}

void TraverseStepper::setDirection(bool clockwise) {
  _dirState = clockwise;
  gpio_set_level(_dirPin, _dirState);
}

void TraverseStepper::setZero() {
  _pos = 0;
}

void TraverseStepper::enableHoming(gpio_num_t pin, bool activeLow) {
  _homePin = pin;
  _homeActiveLow = activeLow;
  _homed = false;
  if(activeLow) {
    pinMode(_homePin, INPUT_PULLUP); // Active low, pull-up
    Serial.println("Homing enabled with active low on pin: " + String(_homePin));
  } else {
    pinMode(_homePin, INPUT_PULLDOWN); // Active high, pull-down
    Serial.println("Homing enabled with active high on pin: " + String(_homePin));
  }

}

void TraverseStepper::checkHome() {
  if (_homePin == GPIO_NUM_NC) return;
  bool triggered = digitalRead(_homePin) == (_homeActiveLow ? LOW : HIGH);
  if (triggered && !_homed) {
    _pos = 0;
    _homed = true;
    Serial.println("Homing triggered! Position reset to 0.");
  } else if (!triggered) {
    _homed = false;
  }
}

bool TraverseStepper::isHomed(){
  return _homed;
}

AxisStepper spindle(SPINDLE_STEP_PIN, SPINDLE_DIR_PIN, 0, SPINDLE_ENABLE_PIN);
TraverseStepper traverse(TRAVERSE_STEP_PIN, TRAVERSE_DIR_PIN, 3, TRAVERSE_ENABLE_PIN);

void initMotionPins() {
  spindle.begin();
  traverse.begin();
}

void initTMC() {
  Serial.println("🔧 Initializing TMC2209...");
  delay(100);  

  TMCSerial.begin(115200, SERIAL_8N1, SPINDLE_RX_PIN, SPINDLE_TX_PIN);            // Match your wiring
                         // Give the driver a moment to wake up

  spindle_driver.begin();                     // SPI/UART comm starts
  spindle_driver.toff(SPINDLE_TOFF);                     // Enables driver (minimum is 1)
  spindle_driver.blank_time(SPINDLE_BLANK_TIME);             // Optional: allows full current pulses
  spindle_driver.rms_current(SPINDLE_RMS_CURRENT);           // Set motor RMS current
  spindle_driver.microsteps(SPINDLE_MICROSTEPS);              // Set your microstep mode (2, 4, 8, 16, etc.)
  spindle_driver.en_spreadCycle(SPINDLE_SPREADCYCLE);      // Use stealthChop (quiet mode)
  spindle_driver.pwm_autoscale(SPINDLE_PWM_AUTOSCALE);        // Automatic current scaling
  spindle_driver.pwm_freq(SPINDLE_PWM_FREQ);                // Optional: change PWM frequency
  spindle_driver.I_scale_analog(SPINDLE_I_AUTOSCALE);      // Set current via UART, not analog pin

  if (spindle_driver.test_connection() == 0) {
    Serial.println("✅ Connected to TMC2209 Spindle via UART");
  } else {
    Serial.println("❌ Failed to connect to TMC2209 Spindle");
  }

  delay(100);
#ifdef DUAL_SERIAL
  TMCSerial_traverse.begin(115200, SERIAL_8N1, TRAVERSE_RX_PIN, TRAVERSE_TX_PIN);
#endif

  traverse_driver.begin();                     // SPI/UART comm starts
  traverse_driver.toff(TRAVERSE_TOFF);                     // Enables driver (minimum is 1)
  traverse_driver.blank_time(TRAVERSE_BLANK_TIME);             // Optional: allows full current
  traverse_driver.rms_current(TRAVERSE_RMS_CURRENT);           // Set motor RMS current
  traverse_driver.microsteps(TRAVERSE_MICROSTEPS);              // Set your microstep
  traverse_driver.en_spreadCycle(TRAVERSE_SPREADCYCLE);      // Use stealthChop (quiet mode)
  traverse_driver.pwm_autoscale(TRAVERSE_PWM_AUTOSCALE);        // Automatic
  traverse_driver.pwm_freq(TRAVERSE_PWM_FREQ);                // Optional: change PWM frequency
  traverse_driver.I_scale_analog(TRAVERSE_I_AUTOSCALE);
  #ifdef SENSORLESS_HOME
  traverse_driver.en_spreadCycle(true);            // Required for StallGuard (disable stealthChop)
  traverse_driver.TCOOLTHRS(0xFFFFF);              // High threshold so StallGuard works at low speed
  traverse_driver.semin(5);                        // CoolStep min
  traverse_driver.semax(2);                        // CoolStep max
  traverse_driver.sedn(0b01);
  traverse_driver.SGTHRS(TRAVERSE_STALLGUARD); // Set StallGuard threshold
  #endif              // StallGuard sensitivity (tune this!)     // Set current

if(traverse_driver.test_connection() == 0) {
    Serial.println("✅ Connected to TMC2209 Traverse via UART");
  } else {
    Serial.println("❌ Failed to connect to TMC2209 Traverse");
  }

}

