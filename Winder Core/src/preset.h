#pragma once
#include <Arduino.h>
#include "global_vars.h"
#include <vector>

struct WinderPreset {
  String name;
  int gauge;
  float width_mm;
  int turns;
  std::vector<int> pattern;
  float length_mm;
  float center_space_mm;
  float overwind_percent;
};

extern WinderPreset presets[8];
void loadDefaultPresets();
