// motion.cpp
#include "motion_HW.h"
#include <driver/ledc.h>

#define R_SENSE 0.11f
#define Spindle_ADDRESS 0b00
#define Traverse_ADDRESS 0b01
#define ADC_MAX 4095
//#define DUAL_SERIAL
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
  if(enableSpindleISR) {

  spindleStepCount++;
  
}}

void IRAM_ATTR onTraverseStep() {
  if(enableTraverseISR) {
    if(traverseDir) {
      traverseStepCount++;
    } else {
      traverseStepCount--;
    }
  }

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
  ledcSetup(_ledcChannel, 0, 1);  // 5 kHz default
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
    enableSpindleISR = run; // Enable or disable ISR based on run state
  }
}
void AxisStepper::getRate(unsigned long timer_ms, int refresh_Time) {
  static unsigned long lastReadTime = 0;
  static float averageRPM = 0;

  // Ring buffer for moving average
  static int rpmBuffer[5] = {0};
  static int bufferIndex = 0;
  static bool bufferFilled = false;

  if (timer_ms - lastReadTime < refresh_Time) return;
  lastReadTime = timer_ms;

  // Read and map pot value
  int potValue = analogRead(SPEED_POT);
  int RPM = map(potValue, 0, ADC_MAX, 0, maxrpm);

  // Store in buffer
  rpmBuffer[bufferIndex] = RPM;
  bufferIndex = (bufferIndex + 1) % speedPotAveraging;
  if (bufferIndex == 0) bufferFilled = true;

  // Compute average
  int sum = 0;
  int count = bufferFilled ? 5 : bufferIndex;
  for (int i = 0; i < count; i++) {
    sum += rpmBuffer[i];
  }
  averageRPM = sum /( count*speedPotChunk);
  averageRPM = (averageRPM *speedPotChunk);
  averageRPM = constrain(averageRPM, 0, maxrpm); // Ensure average is within bounds

  Target_RPM = averageRPM;
}


void AxisStepper::setRate(float RPM) {
  if (RPM > maxrpm) {
    RPM = maxrpm; } // Cap at max RPM
    else if (RPM < 0) {RPM = 0; } // Prevent negative RPM
  Current_RPM = RPM; // Update current RPM
  Current_Step_Rate = (RPM * steps_rev * SPINDLE_MICROSTEPS) / 60.0;
  ledcWriteTone(_ledcChannel, Current_Step_Rate);
}

void AxisStepper::rampAcceleration(int targetRPM, float ramp_rate, bool decelerate, unsigned long timer_ms ,int refresh_Time) {
  static unsigned long lastRampTime = 0;
  if(timer_ms - lastRampTime < refresh_Time) return; // Limit ramping to every 100ms
  lastRampTime = timer_ms;
    if (targetRPM > maxrpm) {targetRPM = maxrpm; } // Cap at max RPM
    else if (targetRPM < 0) {targetRPM = 0; } // Prevent negative RPM
  if(Current_RPM != targetRPM){ // Prevent negative RPM
    if(decelerate)
      {Current_RPM -= (ramp_rate*refresh_Time/1000);
      if(Current_RPM < targetRPM) {Current_RPM = targetRPM; } 
      }
    else
      {Current_RPM += (ramp_rate*refresh_Time/1000);
      if(Current_RPM > targetRPM) {Current_RPM = targetRPM; }}

    setRate(Current_RPM); // Update step rate
  }}


int AxisStepper::getStepCount() const {  // Read the global step count
  return spindleStepCount;
}

float AxisStepper::getTurnCount() const {
  return static_cast<float>(spindleStepCount) / ( steps_rev* SPINDLE_MICROSTEPS);
}

void AxisStepper::resetStepCount() {
  spindleStepCount = 0;
}

TraverseStepper::TraverseStepper(gpio_num_t stepPin, gpio_num_t dirPin, int ledcChannel, gpio_num_t enablePin, gpio_num_t readbackPin)
  : _stepPin(stepPin), _dirPin(dirPin), _enablePin(enablePin), _pos(0),
    _posMin(0), _posMax(1000), _homePin(GPIO_NUM_NC), _homeActiveLow(true), _homed(false), _ledcChannel(ledcChannel) {}

void TraverseStepper::setRPM(int RPM) {
  if (RPM > maxrpm) {
    RPM = maxrpm; } // Cap at max RPM
    else if (RPM < 0) {RPM = 0; } // Prevent negative RPM
    int Step_Rate = (RPM * steps_rev * TRAVERSE_MICROSTEPS) / 60.0;

  ledcWriteTone(_ledcChannel, Step_Rate);
}

void TraverseStepper::begin() {
  pinMode(_dirPin, OUTPUT);
  gpio_set_level(_dirPin, traverseDir);

  if (_enablePin != GPIO_NUM_NC) {
    pinMode(_enablePin, OUTPUT);
    gpio_set_level(_enablePin, HIGH);
  }

  ledcSetup(_ledcChannel, 0, 1);
  ledcAttachPin(_stepPin, _ledcChannel);

  pinMode(TRAVERSE_READBACK_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TRAVERSE_READBACK_PIN), onTraverseStep, RISING);

}

void TraverseStepper::setRate(float stepsPerSecond) {
  ledcWriteTone(_ledcChannel, stepsPerSecond);
}

void TraverseStepper::setEnabled(bool run) {
  if (_enablePin != GPIO_NUM_NC) {
    gpio_set_level(_enablePin, run ? LOW : HIGH);
    enableTraverseISR = run; // Enable or disable ISR based on run state
  }
}

void TraverseStepper::setLimits(int currentpos) {

  if(_holdPos == 0){
    _holdPos = currentpos;
  _posMin = _holdPos + distanceToSteps(_offset+_internal_error);
  _posMax = _holdPos + distanceToSteps(_offset+_width-_internal_error);
  Serial.printf("Min: %d Max: %d\n", _posMin, _posMax);
  }


}

void TraverseStepper::setTraverseRate(int spindleStepRate, float wireGauge)  {
  // Convert spindle step rate to traverse step rate
  int traverseStepRate = spindleStepRate * _multiplier * wireGauge*(TRAVERSE_MICROSTEPS / SPINDLE_MICROSTEPS)/
                          (Lead_Screw_Pitch); // Convert gauge to mm
  
  ledcWriteTone(_ledcChannel, traverseStepRate);

}

void TraverseStepper::controlPosition() {
static bool flip = false; // Flag to reverse direction
if(traverseStepCount >= _posMax && !flip) {
traverseDir = !traverseDir; // Reverse direction if max position exceeded
setDirection(traverseDir);
flip = true; // Set flag to prevent immediate reversal
  Serial.println("Reached max position, reversing direction.");
  _layerCount++; // Increment layer count when max position is reached
} else if(traverseStepCount <= _posMin && flip) {
  flip = false; // Reset flag to allow forward movement again
  traverseDir = !traverseDir; // Reverse direction if min position exceeded
  setDirection(traverseDir);
  Serial.println("Reached min position, reversing direction.");
  _layerCount++; // Increment layer count when max position is reached
} else {
  _pos = traverseStepCount; // Update current position
}
}

int TraverseStepper::getLayerCount() {
  return _layerCount;
}

void TraverseStepper::setPosition(int pos) {
  _pos = pos;
}

int TraverseStepper::getPosition() const {
  return _pos;
}

void TraverseStepper::setDirection(bool clockwise) {
  _dirState = !clockwise;
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

void TraverseStepper::jogDistance(int speed, float distance, bool direction, bool stopAfter) {
  setEnabled(true);
  setDirection(direction);
  setRPM(speed);  // LEDC step pulse output

  int startSteps = traverseStepCount;
  int targetSteps = distanceToSteps(distance);

  while (true) {
    int currentSteps = traverseStepCount;
    int delta = abs(currentSteps - startSteps);
    if (delta >= targetSteps) break;
    delay(1);  // Prevent watchdog timer panic
  }

  if (stopAfter) {
    ledcWriteTone(_ledcChannel, 0);
    setEnabled(false);
  }
}

void TraverseStepper::setMultiplier() {
  float i_multiplier = 1.0f;
  i_multiplier = currentPreset.pattern[_layerCount % currentPreset.pattern.size()];

  if (i_multiplier <= 1.0f) {
    i_multiplier = 1.0f; // Default to 1 if multiplier is invalid
  }
  else if( i_multiplier > MAX_TRAVERSE_MULTIPLIER) {
    i_multiplier = MAX_TRAVERSE_MULTIPLIER; // Cap at maximum multiplier
  }
  _multiplier = i_multiplier;


}

float TraverseStepper::getMultiplier(){
  return _multiplier;
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

void TraverseStepper::compute_backoff(float distance) {
_backoff = distanceToSteps(distance);
  Serial.println("Backoff distance set to " + String(_backoff) + " steps.");
}

void TraverseStepper::UIRunonce(){
  loadCompParameters(currentPreset);
  jogDistance(50,currentPreset.faceplate_thickness,true);
  currentPosition=computePosition();
  totalLayers =  computeLayers();
  calculated_length = computeLength();
}

void TraverseStepper::loadCompParameters(const WinderPreset& preset){
_turns = preset.turns;
_width = preset.width_mm;
_length = preset.length_mm;
_height = preset.center_space_mm;
_internal_error = preset.edge_error;
_gauge_type = preset.gauge;
_gauge = preset.wire_diameter_mm;
_DCR = preset.resistance_per_Meter;
_pattern = preset.pattern;
_offset = preset.faceplate_thickness;
}


int TraverseStepper::computeLayers() {
  int repeat = _pattern.size();
  int consumed_turns = 0;
  int layercount = 0;
  _turnsPerLayer.clear();  // Reset before populating

  while (consumed_turns < _turns) {
    float mult = _pattern[layercount % repeat];
    mult = constrain(mult, 1.0f, MAX_TRAVERSE_MULTIPLIER);

    // Calculate turns in this layer
    int turnlayer = (_width - _internal_error*2) / (_gauge * mult);
    
    if (turnlayer <= 0) break;  // Prevent infinite loop on bad input

    _turnsPerLayer.push_back(turnlayer);

    consumed_turns += turnlayer;
    layercount++;
  }

  _layers = layercount;
  return _layers;
}


int TraverseStepper::computeLength() {
  _lengthPerLayer.clear();
  float totalLength = 0.0f;

  for (size_t i = 0; i < _turnsPerLayer.size(); ++i) {
    int turns = _turnsPerLayer[i];
    float mult = _pattern[i % _pattern.size()];

    // Clamp multiplier for safety
    mult = constrain(mult, 1.0f, MAX_TRAVERSE_MULTIPLIER);

    // Calculate average radius of this layer (height is total bobbin pole diameter)
    float radius = (_height / 2.0f) + i * _gauge;

    // Circumference based on scatter offset
    float circumference = 2.0f * M_PI * radius;

    // Optional: Add scatter compensation fudge factor (small % increase for zig-zag)
    float layerLength = circumference * turns * 1.01f;

    _lengthPerLayer.push_back(layerLength);
    _totalLength += layerLength;
  }

  return static_cast<int>(_totalLength);
  // in mm
}

int TraverseStepper::computeLiveDCR(int currentLayer, int current_steps) {
  if (_lengthPerLayer.empty() || currentLayer < 0) return 0.0f;

  float layerProgress = constrain(((_posMax-_posMin)/(current_steps-_posMin)),0.0f,1.0f);
  

  float totalLength = 0.0f;

  // Sum fully completed layers
  for (int i = 0; i < currentLayer && i < _lengthPerLayer.size(); ++i) {
    totalLength += _lengthPerLayer[i];
  }

  // Add partial progress for current layer
  if (currentLayer < _lengthPerLayer.size()) {
    totalLength += _lengthPerLayer[currentLayer] * layerProgress;
  }

  return static_cast<int>(totalLength * _DCR);  // final DCR value
}


bool TraverseStepper::homeProcess(int rate, bool dir) {
  if (!isHomed()) {
    _backoff = _backoff + plateZeroOffset;
    setEnabled(true);
    setDirection(traverseDir);
    setRPM(rate);
    checkHome();}
    static bool save_steps = false;
    if (isHomed()) {
      if(!save_steps) {
      traverseStepCount = 0; // Reset step count after homing
      save_steps = true;
      traverseDir = !traverseDir; // Reverse direction after homing
      setDirection(traverseDir);} // Back off 10mm}
      if(traverseStepCount >=  _backoff){
        setEnabled(false); // Stop after backoff
        setPosition(0); // Reset position after homing
        setZero(); // Set zero position
        Serial.println("Homing complete! Backed off");
        return true; // Homing successful
    }
  }
  return false; // Homing not yet complete
}

int TraverseStepper::distanceToSteps(float distance) {
  // Convert distance to steps based on your stepper motor's configuration
  //return static_cast<int>(distance * steps_rev * TRAVERSE_MICROSTEPS / Lead_Screw_Pitch);
  return static_cast<int>(distance * steps_rev * (TRAVERSE_MICROSTEPS)/ Lead_Screw_Pitch);
}

float TraverseStepper::computePosition(){
  _stepcount = traverseStepCount;
  return (_stepcount * Lead_Screw_Pitch) / static_cast<float>(steps_rev * TRAVERSE_MICROSTEPS);

  
}
AxisStepper spindle(SPINDLE_STEP_PIN, SPINDLE_DIR_PIN, 0, SPINDLE_ENABLE_PIN, SPINDLE_READBACK_PIN);
TraverseStepper traverse(TRAVERSE_STEP_PIN, TRAVERSE_DIR_PIN, 3, TRAVERSE_ENABLE_PIN, TRAVERSE_READBACK_PIN);

bool homeCycle(int rate, bool dir, float backoffDistance, unsigned long timer_ms, int refresh_Time) {
  static bool runonce = false;
  static bool homed = false;
  static unsigned long lastHomingTime = 0;
  static bool skip = false;
  if (timer_ms - lastHomingTime < refresh_Time) { return homed;} // Limit homing to every 100ms
  lastHomingTime = timer_ms;

  if(!runonce) {
    Serial.println("🔧 Homing cycle initiated...");
    runonce = true;
    
  }

  if(!skip){
  homed = traverse.homeProcess(rate, dir);}

  if(homed) {
    Serial.println("✅ Homing cycle completed successfully!");
    traverse.setEnabled(false); // Stop traverse after homing
    traverse.setZero(); // Set zero position after homing
    skip = true;}
  return homed;

}

void initMotionPins() {
  pinMode(SPEED_POT,INPUT);
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

