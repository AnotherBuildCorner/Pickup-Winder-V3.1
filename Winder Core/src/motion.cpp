// motion.cpp
#include "motion.h"
#include "global_vars.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp32-hal-timer.h"

#include <TMCStepper.h>

#define R_SENSE 0.11f
#define DRIVER_ADDRESS 0b00  // Default if not using address jumpers

HardwareSerial& TMCSerial = Serial1;  // Or Serial2 depending on wiring
TMC2209Stepper driver(&TMCSerial, R_SENSE, DRIVER_ADDRESS);

#define STEP_PIN 25
volatile bool step_state = false;
#define ESP_TIMER_ISR (esp_timer_dispatch_t)0

volatile int spindle_isr_count = 0;
hw_timer_t* timer = nullptr;

class AxisStepper {
public:
  AxisStepper(gpio_num_t stepPin, gpio_num_t dirPin, gpio_num_t enablePin = GPIO_NUM_NC)
    : _stepPin(stepPin), _dirPin(dirPin), _enablePin(enablePin), _accum(0),
      _targetRate(0), _currentRate(0), _accelPerTick(0),
      _stepState(false), _run(false), _dirState(true) {}

  void begin(bool clockwise = true) {
    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);
    gpio_set_level(_stepPin, 0);
    gpio_set_level(_dirPin, _dirState);

    if (_enablePin != GPIO_NUM_NC) {
      pinMode(_enablePin, OUTPUT);
      gpio_set_level(_enablePin, !_run);  // LOW = enabled on many drivers
    }

    setDirection(clockwise);
  }

  void setDirection(bool clockwise) {
    _dirState = clockwise;
    gpio_set_level(_dirPin, _dirState);
  }

  void setEnabled(bool run) {
    _run = run;
    if (_enablePin != GPIO_NUM_NC) {
      gpio_set_level(_enablePin, !_run);  // Assume LOW = enabled
    }
  }

  void setRate(float stepsPerSecond) { _targetRate = stepsPerSecond; }

  void setAcceleration(float stepsPerSec2) {
    _accelPerTick = (stepsPerSec2 * SCALE) / (ISR_FREQ * ISR_FREQ);
  }

  void tick() {
    if (!_run) return;

    // acceleration ramp
    if (_currentRate < _targetRate) {
      _currentRate += _accelPerTick;
      if (_currentRate > _targetRate) _currentRate = _targetRate;
    } else if (_currentRate > _targetRate) {
      _currentRate -= _accelPerTick;
      if (_currentRate < _targetRate) _currentRate = _targetRate;
    }

    uint32_t increment = (_currentRate * SCALE) / ISR_FREQ;
    _accum += increment;

    if (_accum >= SCALE) {
      _accum -= SCALE;
      gpio_set_level(_stepPin, _stepState ? 0 : 1);
      _stepState = !_stepState;
    }
  }

private:
  gpio_num_t _stepPin, _dirPin, _enablePin;
  uint32_t _accum;
  float _targetRate, _currentRate, _accelPerTick;
  bool _stepState, _run, _dirState;

  static constexpr uint32_t SCALE = 1000000;
  static constexpr uint32_t ISR_FREQ = 100000;
};



class TraverseStepper {
public:
  TraverseStepper(gpio_num_t stepPin, gpio_num_t dirPin, gpio_num_t enablePin)
    : _stepPin(stepPin), _dirPin(dirPin), _accum(0), _rate(0),
      _stepState(false), _dirState(true), _run(false),_pos(0), _posMin{0}, _posMax{1000} {}

  void begin() {
    pinMode(_stepPin, OUTPUT);
    pinMode(_dirPin, OUTPUT);
    gpio_set_level(_stepPin, 0);
    gpio_set_level(_dirPin, _dirState);
  }

  void setRate(float stepsPerSecond) {
    _rate = stepsPerSecond;
    _increment = (_rate * SCALE) / ISR_FREQ;
  }

  void setLimits(int minPos, int maxPos) {
    _posMin = minPos;
    _posMax = maxPos;
  }

  void tick() {
  if (!_run) return;

  checkHome();  // ✅ now active
    _accum += _increment;

    if (_accum >= SCALE) {
      _accum -= SCALE;

      // Step pulse
      gpio_set_level(_stepPin, _stepState ? 0 : 1);
      _stepState = !_stepState;

      if (_stepState == false) { // Only count one edge
        _pos += _dirState ? 1 : -1;

        // Check for reversal
    if (_pos >= _posMax) {
      _dirState = false;
      gpio_set_level(_dirPin, _dirState);
    } else if (_pos <= _posMin) {
      _dirState = true;
      gpio_set_level(_dirPin, _dirState);
    }
      }
    }
  }
    void setPosition(int pos) { _pos = pos; }
  int getPosition() const { return _pos; }
  void setZero() { _pos = 0; }

  void enableHoming(gpio_num_t pin, bool activeLow = true) {
    _homePin = pin;
    _homeActiveLow = activeLow;
    pinMode(_homePin, INPUT);
  }

private:
  gpio_num_t _stepPin, _dirPin;
  uint32_t _accum, _increment;
  float _rate;
  bool _stepState, _dirState, _run;
  int _pos, _posMin, _posMax;

  static constexpr uint32_t SCALE = 1000000;
  static constexpr uint32_t ISR_FREQ = 100000;

   gpio_num_t _homePin = GPIO_NUM_NC;
  bool _homeActiveLow = true;

  void checkHome() {
    if (_homePin == GPIO_NUM_NC) return;

    bool triggered = digitalRead(_homePin) == (_homeActiveLow ? LOW : HIGH);
    if (triggered && !_homed) {
      _pos = 0;
      _homed = true;
    } else if (!triggered) {
      _homed = false;
    }
  }

  bool _homed = false;  // debounce tracker
};



AxisStepper spindle(SPINDLE_STEP_PIN, SPINDLE_DIR_PIN, SPINDLE_ENABLE_PIN);
TraverseStepper traverse(TRAVERSE_STEP_PIN, TRAVERSE_DIR_PIN, TRAVERSE_ENABLE_PIN);



void IRAM_ATTR onTimer() {
  spindle.tick();     // internally skips if disabled
  traverse.tick();    // same
}



void initTMC(){
  TMCSerial.begin(115200); // Match TMC UART speed

  driver.begin();
  driver.toff(4);
  driver.rms_current(600);     // mA
  driver.microsteps(16);

  if (driver.test_connection() == 0) {
    Serial.println("✅ Connected to TMC2209");
  } else {
    Serial.println("❌ No response from TMC2209");
  }
}
void initMotionPins() {
  pinMode((gpio_num_t)SPINDLE_STEP_PIN, OUTPUT);
  gpio_set_level((gpio_num_t)SPINDLE_STEP_PIN, LOW);

  timer = timerBegin(0, 80, true);          // 80MHz / 80 = 1 µs ticks
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1, true);          // 🔥 1 µs interval
  timerAlarmEnable(timer);

}




//esp_timer_handle_t step_timer;
/*
void IRAM_ATTR stepper_isr(void* arg) {
  if(spindle_isr_count >= spindle_isr_mult){
    spindle_isr_count = 0; // reset counter
    step_state = !step_state;
    gpio_set_level((gpio_num_t)SPINDLE_STEP_PIN, step_state);
    if (!step_state && steps_remaining > 0 && run == true) {
    steps_remaining--;
    if (steps_remaining == 0) {
      esp_timer_stop(step_timer);
    }
  }
  } else {
    spindle_isr_count++;
  }

} */

/*
  const esp_timer_create_args_t step_timer_args = {
    .callback = &stepper_isr,          // just the function pointer
    .arg = NULL,                       // or a pointer to a config struct
    .dispatch_method = ESP_TIMER_ISR,  // run ISR directly
    .name = "stepper"
  };
  esp_timer_create(&step_timer_args, &step_timer);
  esp_timer_start_periodic(step_timer, ISR_delay); // microseconds */