#include "global_vars.h"
#pragma once

#include <map>
#include <string>

struct WireProperties {
    float diameter_mm;          // grams
    float minTension_g;         // grams
    float maxTension_g; 
    float breakStrength_g;         // grams
    float resistancePerMeter;   // ohms/m
           // mm
};

bool applyWireProperties(WinderPreset& presets);