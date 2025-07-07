#pragma once
#include <Arduino.h>
#include "global_vars.h"
#include "FFat.h"
#include "SPIFFS.h"



void loadDefaultPresets();
void bootFS();
void savePresetToFile(const WinderPreset& preset, const char* filename);
WinderPreset loadPresetFromReadableFile(const char* filename);
std::vector<WinderPreset> loadAllPresets();