#include "HX711.h"
#include "global_vars.h"
#include "tensioner.h"

float scaleFactor = testread / testweight;
float Tensioner_Max_Reading = 0;
#define Tensioner_Peak_Reads 10
HX711 scale;

void tensioner_setup() {

  scale.begin(TENSIONER_READ_PIN, TENSIONER_SCK_PIN);
  Serial.println("Taring... remove all weight.");
  scale.set_scale(scaleFactor);      // Optional: set a scale factor later
  scale.tare();
  if(scale.is_ready()){
    Serial.println("HX711 is ready");
  } else {
    Serial.println("HX711 not ready, check connections");
  }           // Zero the scale
}

void tensioner_output(unsigned long timer_ms, int refresh_Time) {
  static unsigned long lastReadTime = 0;
  static int maxcounter = 0;
  if (timer_ms - lastReadTime < refresh_Time) return;
  maxcounter++;
  if(maxcounter >=Tensioner_Peak_Reads){
    maxcounter = 0;
    Tensioner_Max_Reading = 0;
  }

  lastReadTime = timer_ms;
  bool printserial = false;
  bool nr = true;
  Tensioner_reading = 1111.1111;
  if (scale.is_ready()) {
    Tensioner_reading = scale.get_units(1);  // Or .read_average() for raw
    Tensioner_Max_Reading = max(Tensioner_reading,Tensioner_Max_Reading);
    nr = false;}

    if(printserial){
      if(!nr){
    Serial.println("HX711 not ready");
      }
      else{
    Serial.print("Tension: ");
    Serial.print(Tensioner_reading);
    Serial.println(" g");  // Print in grams
    }
  }}



