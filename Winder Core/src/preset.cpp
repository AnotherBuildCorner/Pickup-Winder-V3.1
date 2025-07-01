#include "preset.h"
#include "global_vars.h"
WinderPreset presets[8];

void loadDefaultPresets() {
WinderPreset defaults[8] = {
  {
    "Strat",
    7600,
    42,
    false,
    52.0,
    0.0,
    {2, 4, 6, 0},
    48.0,
    1.2,
    2.0,
    1.0
  },
  {
    "Tele",
    8000,
    43,
    false,
    50.0,
    0.0,
    {1, 3, 5, 0},
    49.0,
    1.0,
    2.0,
    1.0
  },
  {
    "Humbucker",
    5000,
    44,
    false,
    60.0,
    0.0,
    {5, 5, 5, 0},
    40.0,
    2.0,
    2.0,
    1.0
  },
  {
    "Jazz Bass",
    9000,
    45,
    false,
    55.5,
    0.0,
    {4, 2, 1, 0},
    51.0,
    1.5,
    2.0,
    1.0
  },
  {
    "P90",
    10000,
    42,
    false,
    53.2,
    0.0,
    {3, 3, 6, 0},
    47.5,
    2.5,
    2.0,
    1.0
  },
  {
    "FilterTron",
    8200,
    44,
    false,
    49.9,
    0.0,
    {1, 2, 3, 0},
    46.0,
    1.0,
    2.0,
    1.0
  },
  {
    "MiniHB",
    8700,
    43,
    false,
    48.0,
    0.0,
    {6, 3, 1, 0},
    50.0,
    2.2,
    2.0,
    1.0
  },
  {
    "Custom",
    1000,
    42,
    false,
    56.0,
    0.0,
    {2, 2, 2, 7, 0},
    53.0,
    1.0,
    2.0,
    1.0
  }
};

  for (int i = 0; i < 8; i++) {
    presets[i] = defaults[i];
  }
}