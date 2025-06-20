#include "preset.h"
#include "global_vars.h"
WinderPreset presets[8];

void loadDefaultPresets() {
    WinderPreset defaults[8] = {
  {
    "Strat",         // name
    42,              // gauge
    52.0,            // width_mm
    7600,            // turns
    {2, 4, 6, 0},    // pattern
    48.0,            // length_mm
    1.2,             // center_space_mm
    0.0              // overwind_percent
  },
  {
    "Tele",
    43,
    50.0,
    8000,
    {1, 3, 5, 0},
    49.0,
    1.0,
    0.0
  },
  {
    "Humbucker",
    44,
    60.0,
    5000,
    {5, 5, 5, 0},
    40.0,
    2.0,
    10.0
  },
  {
    "Jazz Bass",
    45,
    55.5,
    9000,
    {4, 2, 1, 0},
    51.0,
    1.5,
    5.0
  },
  {
    "P90",
    42,
    53.2,
    10000,
    {3, 3, 6, 0},
    47.5,
    2.5,
    -5.0
  },
  {
    "FilterTron",
    44,
    49.9,
    8200,
    {1, 2, 3, 0},
    46.0,
    1.0,
    0.0
  },
  {
    "MiniHB",
    43,
    48.0,
    8700,
    {6, 3, 1, 0},
    50.0,
    2.2,
    8.0
  },
  {
    "Custom",
    41,
    56.0,
    10500,
    {2, 2, 2, 2, 0},
    53.0,
    1.0,
    0.0
  }
  };

  // Copy to global array
  for (int i = 0; i < 8; i++) {
    presets[i] = defaults[i];
  }
}