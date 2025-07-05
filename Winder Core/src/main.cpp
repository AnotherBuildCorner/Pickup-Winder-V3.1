#include <Arduino.h>
#include "preset.h"
#include "ui.h"
#include <TFT_eSPI.h>
//#include "motion.h"
#include "motion_HW.h"
#include "tensioner.h"
#include "main.h"

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
    traverse.compute_backoff(5.0f); // Set backoff distance to 10mm
    currentPreset=presets[0];


}


void loop() {
  

  static unsigned long screentime = 0;
  static unsigned long speedtimer = 0;
  static unsigned long coretimer = 0; // initial speed in microseconds per step
  unsigned long ct = micros();
  unsigned long ct2 = millis();
  static unsigned long traverse_timer = 0;
  static bool direction = 1;
  static bool homed = false;
  static bool runonce = false;
  if(!homed){homed = homeCycle(50, traverseDir, 5.0f, ct2,100);}
    if(launchActive){
      if(!run){
        runonce = false;
      }

      timer_core_ms(ct2, 100); // Update core timer every 100ms
      windingScreenHandler(ct2, 100); // Update the winding screen every 100ms

      if(spindle.getTurnCount() >= presets[selectedPreset].turns){
        run = false; }// Stop winding after 5000 turns}
      
      spindle.setEnabled(run);
      traverse.setEnabled(run);

        
      if(run && homed){ // Read the speed pot value and update target RPM
        if(!runonce){
          spindleStepCount = 0;
          traverseStepCount = 0;
          runonce = true;
          traverse.setDirection(traverseDir); // Set initial direction
        }
        ramp_handler(ct2, 100); // Handle spindle ramping every 100ms
        traverse_handler(ct2, 100); // Handle traverse movement every 100ms
        

  }}
  else{updateUI();
  tensioner_output(ct2, 100);}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void timer_core_ms(unsigned long timer_ms, int refresh_Time) {
  static unsigned long lastCoreTime = 0;
  if (timer_ms - lastCoreTime < refresh_Time) return; // Limit core updates to every 100ms
  lastCoreTime = timer_ms;
  turn_count = spindle.getTurnCount();
  tensioner_output(timer_ms, refresh_Time); // Update tensioner output

}


void ramp_handler(unsigned long timer_ms, int refresh_Time) {
        static unsigned long lastRampTime = 0;
        if(timer_ms - lastRampTime < refresh_Time) return; // Limit ramping to every 100ms

        if(spindle.getTurnCount() > currentPreset.turns-spin_down_turns){spindle.rampAcceleration(100, Accel_RPM, true, timer_ms, refresh_Time);}
        else{spindle.rampAcceleration(Target_RPM, Accel_RPM, false, timer_ms, refresh_Time);}
        
         // Set traverse rate based on spindle speed and wire gauge
}


void traverse_handler(unsigned long timer_ms, int refresh_Time) {
  static unsigned long lastTraverseTime = 0;
  spindle.getRate(timer_ms, refresh_Time); // Read the speed pot value and update target RPM
  if (timer_ms - lastTraverseTime < refresh_Time) return; // Limit traverse updates to every 100ms
    lastTraverseTime = timer_ms;
    traverse.setLimits(0,10);
    traverse.setTraverseRate(Current_Step_Rate, .0635, 1);
    Serial.printf("Traverse Steps : %d\n", traverseStepCount);
    traverse.controlPosition(); // Control position based on step count

}

void traverse_handler_old(unsigned long timer_ms, int refresh_Time) {
  static unsigned long lastTraverseTime = 0;
  static unsigned long traverse_step_counter = 0;
  static bool traverse_direction = true; // true = forward, false = backward

  spindle.getRate(timer_ms, refresh_Time); // Read the speed pot value and update target RPM

  if (timer_ms - lastTraverseTime < refresh_Time) return; // Limit traverse updates to every 100ms
    lastTraverseTime = timer_ms;

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