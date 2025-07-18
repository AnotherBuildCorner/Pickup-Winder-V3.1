#pragma once
#include <Arduino.h>
#include "global_vars.h"
#include "SPIFFS.h"
#include <SD.h>


void loadDefaultPresets();
void bootFS();
void initSD();
void initSPIFF();

void savePresetToFile(const WinderPreset& preset, const char* filename);
WinderPreset loadPresetFromReadableFile(const char* filename);
std::vector<WinderPreset> loadAllPresets();

void savePresetToFileSPIFF(const WinderPreset& preset, const char* filename);
WinderPreset loadPresetFromReadableFileSPIFF(const char* filename);
std::vector<WinderPreset> loadAllPresetsSPIFF();