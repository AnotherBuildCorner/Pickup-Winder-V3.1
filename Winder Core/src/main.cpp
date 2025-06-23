#include <Arduino.h>
#include "preset.h"
#include "ui.h"
#include <TFT_eSPI.h>
#include "motion.h"
#include "tensioner.h"

#define enable_screen false // Set to false to disable screen updates
int calibrationDone = 1;
void setup() {
  Serial.begin(115200);
  initTMC();

  Serial.println("Did we boot?");
  


  loadDefaultPresets();   // Loads dummy preset data
  if(enable_screen){
  initUI();     
  if(!calibrationDone) {
    runCalibration();      // Run calibration only if not done
  }}

    initMotionPins();
    tensioner_setup();
    
launchActive = true;
run = true;
}


void loop() {
  static unsigned long screentime = 0;
  static unsigned long speedtimer = 0;
  static unsigned long coretimer = 0; // initial speed in microseconds per step
  unsigned long ct = micros();
  unsigned long ct2 = millis();
  static bool direction = 1;



    if(launchActive){
      delay(100);
      tensioner_output();
      run = true;
      if(screentime + refreshmicros < ct && enable_screen) {screentime = ct; drawWindingScreen();}
      if(coretimer + 5000 < ct2){turns = steps_remaining / steps_rev; // controls screen refresh rate
        coretimer = ct2;
        direction = !direction;
        digitalWrite(SPINDLE_DIR_PIN, direction);
        

        Serial.println(direction);} // Set spindle direction
        
      
     
     // controls screen refresh rate
      if(speedtimer + speed < ct){
          speedtimer = ct;
          Current_RPM += accel; // increase speed every time the timer expires
      if(Current_RPM > maxrpm) Current_RPM = maxrpm; // cap speed
        speed = RPM_to_micros / Current_RPM; // convert RPM to microseconds per step

      //Serial.println("step: " + String(steps_traversed) + " turns: " + String(turns) + " speed: " + String(speed));
    }
  }
  else
  {
  
    updateUI();
  }
}
