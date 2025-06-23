#include "HX711.h"
#include "global_vars.h"
#include "tensioner.h"

float scaleFactor = testread / testweight;
HX711 scale;

void tensioner_setup() {

  scale.begin(TENSIONER_READ_PIN, TENSIONER_SCK_PIN);

  Serial.println("Taring... remove all weight.");
  scale.set_scale(scaleFactor);      // Optional: set a scale factor later
  scale.tare();           // Zero the scale

  Serial.println("Scale ready");
}

void tensioner_output() {
  if (scale.is_ready()) {
    float reading = scale.get_units(1);  // Or .read_average() for raw
    Serial.print("Tension: ");
    Serial.print(reading);
    Serial.println(" g");  // Print in grams
  } else {
    Serial.println("HX711 not ready");
  }

}
