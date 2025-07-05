#include "global_vars.h"
#include "wire_parameters.h"

std::map<std::string, WireProperties> wireTable = {
    { "42n", { 0.0635f, 17.0f, 22.0f, 6.5f  } },   // Example values
    { "42h",  { 0.0635f, 17.0f, 22.0f, 6.5f  } },
    { "44n", { 0.0635f, 17.0f, 22.0f, 6.5f   } },
    // Add more gauges/types as needed
};

#define break_strength_42 80 //grams // Break strength in g
#define min_42 17 // grams winding tension
#define max_42 22
#define diameter_42_std 0.065
#define DCR_mm_42 .0000015 //mm

bool applyWireProperties(WinderPreset& preset) {
  auto it = wireTable.find(preset.gauge);
  if (it != wireTable.end()) {
    const WireProperties& props = it->second;
    preset.wire_diameter_mm   = props.diameter_mm;
    preset.min_tension_g      = props.minTension_g;
    preset.max_tension_g      = props.maxTension_g;
    preset.break_strength_g   = props.breakStrength_g;
    preset.resistance_per_Meter = props.resistancePerMeter;
    return true;
  } else {
    Serial.printf("Wire gauge '%s' not found in wire table\n", preset.gauge.c_str());
    return false;
  }
}
