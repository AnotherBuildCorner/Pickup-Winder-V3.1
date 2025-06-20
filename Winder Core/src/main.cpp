#include <Arduino.h>
#include "preset.h"
#include "ui.h"
#include <TFT_eSPI.h>
int calibrationDone = 1;
void setup() {
  Serial.begin(115200);

  loadDefaultPresets();   // Loads dummy preset data
  initUI();     
  if(!calibrationDone) {
    runCalibration();      // Run calibration only if not done
  }

}


void loop() {
  static unsigned long screentime = 0;
  static unsigned long speedtimer = 0;
  static unsigned long ct = micros();
  static int speed = 0;


  if(launchActive){
  if(screentime+refreshmicros < ct) {screentime = ct; updateUI(); } // controls screen refresh rate
  if(speedtimer+speed < ct){
    speedtimer = ct;
    speed += accel_delay_step; // increase speed every time the timer expires
    if(speed > speed_timer_max) speed = speed_timer_max; // cap speed
  }
  }
  else{
    updateUI();
  }
}
