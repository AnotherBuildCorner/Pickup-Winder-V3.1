#pragma once

#include <Arduino.h>
#include "preset.h"
#include "config.h"
#include "wire_parameters.h"
#include <vector>

extern volatile bool tick;

extern volatile bool launchActive;
extern  unsigned long steps_traversed;
extern  int turn_count;
extern unsigned long speed;
extern unsigned long Current_Step_Rate;
extern int Current_RPM;

extern int selectedPreset;

extern bool runflag;
extern int Current_RPM;
extern int Target_RPM; // Target RPM for the spindle
extern bool run;

extern volatile int steps_remaining; // -1 = run forever
extern volatile int spindle_isr_mult; // multiplier for ISR to control step frequency

extern unsigned long tick_count; // Current winding layer
extern volatile bool tick; // Used to synchronize with the main loop
extern float Tensioner_reading; // Current tensioner reading in grams

extern volatile uint32_t spindleStepCount;

struct WinderPreset {
  String name;
    int turns;
  int gauge;
  bool Spin_direction; // true = clockwise, false = counter-clockwise
  float width_mm;
  float overwind_percent;
  std::vector<int> pattern;
  float length_mm;
  float center_space_mm;
  float faceplate_thickness;
  float edge_error;
   // Optional, default 0
};

extern WinderPreset presets[8];

extern WinderPreset currentPreset;