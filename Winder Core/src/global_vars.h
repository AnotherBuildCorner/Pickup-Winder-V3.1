#pragma once

#include <Arduino.h>
#include "config.h"
#include <vector>

extern volatile bool tick;

extern volatile bool launchActive;
extern  unsigned long steps_traversed;
extern  int turn_count;
extern unsigned long speed;
extern unsigned long Current_Step_Rate;
extern int Current_RPM;

extern int selectedPreset;
extern int currentLayer;
extern int current_DCR;
extern int calculated_length;
extern int totalLayers; // Total number of layers in the winding
extern bool runflag;
extern int Current_RPM;
extern int Target_RPM; // Target RPM for the spindle
extern float currentPosition; // Current position of the traverse stepper
extern bool run;
extern float current_multiplier;
extern int MAX_dcr;

extern volatile int steps_remaining; // -1 = run forever
extern volatile int spindle_isr_mult; // multiplier for ISR to control step frequency

extern unsigned long tick_count; // Current winding layer
extern volatile bool tick; // Used to synchronize with the main loop
extern float Tensioner_reading; // Current tensioner reading in grams
extern float Tensioner_Max_Reading;

extern volatile uint32_t spindleStepCount;
extern volatile int traverseStepCount;
extern volatile bool traverseDir; // true = forward, false = backward
extern volatile bool enableTraverseISR;
extern volatile bool enableSpindleISR;
extern float plateZeroOffset;


extern std::vector<int> precomputedMultipliers;

struct WinderPreset {
  String name;
  int turns;
  std::string gauge;
  bool Spin_direction; // true = clockwise, false = counter-clockwise
  float width_mm;
  float overwind_percent;
  std::vector<float> pattern;
  float length_mm;
  float center_space_mm;
  float faceplate_thickness;
  float edge_error;
  float wire_diameter_mm;
  float resistance_per_Meter;
  float min_tension_g;
  float max_tension_g;
  float break_strength_g;

   // Optional, default 0
};

extern WinderPreset presets[8];
extern WinderPreset currentPreset;

    
