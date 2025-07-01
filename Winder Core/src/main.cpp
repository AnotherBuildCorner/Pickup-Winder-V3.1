#include <Arduino.h>
#include "preset.h"
#include "ui.h"
#include <TFT_eSPI.h>
//#include "motion.h"
#include "motion_HW.h"
#include "tensioner.h"

#define enable_screen true // Set to false to disable screen updates
int calibrationDone = 1;
void setup() {
  Serial.begin(115200);
  

  delay(500);
  Serial.println("Did we boot?");
  
  loadDefaultPresets();   // Loads dummy preset data
  if(enable_screen){
  initUI();     
  if(!calibrationDone) {
    runCalibration();      // Run calibration only if not done
  }}


    initMotionPins();
    delay(100);
    
    tensioner_setup();
    delay(100);
    initTMC();
    delay(100);
    traverse.enableHoming(DIAG_PIN, false); // Enable homing with active low

}


void loop() {
  

  static unsigned long screentime = 0;
  static unsigned long speedtimer = 0;
  static unsigned long coretimer = 0; // initial speed in microseconds per step
  unsigned long ct = micros();
  unsigned long ct2 = millis();
  static unsigned long traverse_timer = 0;
  static bool direction = 1;
  static bool homed = true;
  static bool runonce = false;




    if(launchActive){
      static unsigned long traverse_step_counter = 0;
      static bool traverse_direction = true; // true = forward, false = backward
      run = true;
      

      if(spindle.getTurnCount() >= presets[selectedPreset].turns){
        run = false; // Stop winding after 5000 turns
        spindle.setEnabled(false);
        traverse.setEnabled(false);}
 // Reset launch flag
    if(run && homed == false){
      if(runonce == false){
        runonce = true;
        traverse.setDirection(true);
        traverse.setEnabled(true);
        
        traverse.setRate(500);
        
        
    }
        tensioner_output(ct2, 100);
        traverse.checkHome(); // Check if homing is triggered
        if(traverse.isHomed()) {
          //homed = true;
          traverse.setPosition(0); // Reset position after homing
          traverse.setZero(); // Reset the stepper position
          traverse.setEnabled(false); // Disable traverse after homing
          delay(1000);

        }

  }


      
      if(run && homed){
        //Target_RPM = 1000;
        spindle.getRate(ct2, 100); // Read the speed pot value and update target RPM
        spindle.setEnabled(true);
        traverse.setEnabled(true); // Disable traverse after homing

        tensioner_output(ct2, 100);

        if(spindle.getTurnCount() > presets[selectedPreset].turns-spin_down_turns){spindle.rampAcceleration(100, Accel_RPM, true, ct2, 100);}
        else{spindle.rampAcceleration(Target_RPM, Accel_RPM, false, ct2, 100);}


         // Ramp acceleration
        


        if(traverse_timer + 200 < ct2){
          
          traverse_timer = ct2;
          traverse.setRate(Current_RPM);
          if(traverse_direction){
            traverse_step_counter+= 1; // Current_Step_Rate/5;
            if(traverse_step_counter >= 20) { // Change direction every 1000 steps  5*200*TRAVERSE_MICROSTEPS
              traverse_direction = false;
              traverse.setDirection(false);
            }
          } else {
            traverse_step_counter-= 1; // Current_Step_Rate/5;;
            if(traverse_step_counter <= 0) { // Change direction every 1000 steps
              traverse_direction = true;
              traverse.setDirection(true);
            }
          }
        }

  
      if(screentime + refreshmicros < ct && enable_screen) {screentime = ct; drawWindingScreen();}
      if(coretimer + 200*1000 < ct){
        turn_count = spindle.getTurnCount(); 
        coretimer = ct;

    }
  }}
  else{updateUI();
  tensioner_output(ct2, 100);}
}

