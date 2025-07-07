#include "preset.h"
#include "global_vars.h"
#include "wire_parameters.h"

#include <Arduino.h>
#include "FFat.h"
#include "USBMSC.h"


USBMSC MSC;
#include "USB.h"
WinderPreset presets[8];

void loadDefaultPresets() {
WinderPreset defaults[8] = {
  {
    "Strat",
    7600,
    "42n",
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
    "42h",
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
    "Humbucker",  //name
    5000,  //turns
    "42n",  //gauge marker
    false, //turn dir  false = ccw
    6.0, //width
    0.0, //overwind
    {1, 2, 2, 5, 1,1}, //pattern
    57.5, //length mm
    6.5, //center space mm
    2.0, // faceplate offset
    1.0 // edge error
  },
  {
    "Jazz Bass",
    9000,
    "42n",
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
    "42n",
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
    "42n",
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
    "42n",
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
    1000,  //turns
    "42n",  //gauge marker
    false, //turn dir  false = ccw
    6.0, //width
    5.0, //overwind %
    {1, 2, 2, 5, 1,1}, //pattern
    57.5, //length mm
    6.5, //center space mm
    2.0, // faceplate offset
    1.0 // edge error
  }
};

  for (int i = 0; i < 8; i++) {
    presets[i] = defaults[i];
    applyWireProperties(presets[i]);
  }
}




WinderPreset loadPresetFromReadableFile(const char* filename) {
  File file = FFat.open(filename, "r");  // or LittleFS.open()
  WinderPreset preset;

  if (!file) {
    Serial.printf("Failed to open file: %s\n", filename);
    return preset;
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0 || line.startsWith("#")) continue;  // Skip empty lines or comments

    int sep = line.indexOf(':');
    if (sep == -1) continue;

    String key = line.substring(0, sep);
    String value = line.substring(sep + 1);
    key.trim();
    value.trim();

    if (key == "name") preset.name = value;
    else if (key == "turns") preset.turns = value.toInt();
    else if (key == "gauge") preset.gauge = value.c_str();  // string → std::string
    else if (key == "spin_direction") preset.Spin_direction = (value == "true" || value == "1");
    else if (key == "width_mm") preset.width_mm = value.toFloat();
    else if (key == "overwind_percent") preset.overwind_percent = value.toFloat();
    else if (key == "pattern") {
      preset.pattern.clear();
      int start = 0;
      while (start < value.length()) {
        int comma = value.indexOf(',', start);
        if (comma == -1) comma = value.length();
        preset.pattern.push_back(value.substring(start, comma).toFloat());
        start = comma + 1;
      }
    }
    else if (key == "length_mm") preset.length_mm = value.toFloat();
    else if (key == "center_space_mm") preset.center_space_mm = value.toFloat();
    else if (key == "faceplate_thickness") preset.faceplate_thickness = value.toFloat();
    else if (key == "edge_error") preset.edge_error = value.toFloat();
    else if (key == "wire_diameter_mm") preset.wire_diameter_mm = value.toFloat();
    else if (key == "resistance_per_Meter") preset.resistance_per_Meter = value.toFloat();
    else if (key == "min_tension_g") preset.min_tension_g = value.toFloat();
    else if (key == "max_tension_g") preset.max_tension_g = value.toFloat();
    else if (key == "break_strength_g") preset.break_strength_g = value.toFloat();
  }

  file.close();
  return preset;
}


std::vector<WinderPreset> loadAllPresets() {
  std::vector<WinderPreset> all;
  for (int i = 0; i < 8; ++i) {
    String path = "/preset_" + String(i) + ".txt";
    all.push_back(loadPresetFromReadableFile(path.c_str()));
  }
  return all;
}


void savePresetToFile(const WinderPreset& preset, const char* filename) {
  File file = FFat.open(filename, "w");  // Or LittleFS.open() if you're using LittleFS
  if (!file) {
    Serial.printf("Failed to open file for writing: %s\n", filename);
    return;
  }

  file.printf("name: %s\n", preset.name.c_str());
  file.printf("turns: %d\n", preset.turns);
  file.printf("gauge: %s\n", preset.gauge.c_str());
  file.printf("spin_direction: %s\n", preset.Spin_direction ? "true" : "false");
  file.printf("width_mm: %.3f\n", preset.width_mm);
  file.printf("overwind_percent: %.3f\n", preset.overwind_percent);

  // Write pattern as comma-separated list
  file.print("pattern: ");
  for (size_t i = 0; i < preset.pattern.size(); ++i) {
    file.print(preset.pattern[i], 3);
    if (i < preset.pattern.size() - 1) file.print(", ");
  }
  file.println();

  file.printf("length_mm: %.3f\n", preset.length_mm);
  file.printf("center_space_mm: %.3f\n", preset.center_space_mm);
  file.printf("faceplate_thickness: %.3f\n", preset.faceplate_thickness);
  file.printf("edge_error: %.3f\n", preset.edge_error);
  file.printf("wire_diameter_mm: %.5f\n", preset.wire_diameter_mm);
  file.printf("resistance_per_Meter: %.3f\n", preset.resistance_per_Meter);
  file.printf("min_tension_g: %.1f\n", preset.min_tension_g);
  file.printf("max_tension_g: %.1f\n", preset.max_tension_g);
  file.printf("break_strength_g: %.1f\n", preset.break_strength_g);

  file.close();
  Serial.printf("Saved preset to %s\n", filename);
}
