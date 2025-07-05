#include <Arduino.h>
#include "ui.h"
#include <TFT_eSPI.h>
//#include "motion.h"
//#include "motion_HW.h"  // inside the ui.h
#include "wire_parameters.h"
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

}


void loop() {
  

  static unsigned long screentime = 0;
  static unsigned long speedtimer = 0;
  static unsigned long coretimer = 0; // initial speed in microseconds per step
  static unsigned long serialtimer = 0;
  unsigned long ct = micros();
  unsigned long ct2 = millis();
  static unsigned long traverse_timer = 0;
  static bool direction = 1;
  static bool homed = false;
  static bool runonce = false;

  if(serialtimer + 1000 < ct2) {
    serialtimer = ct2;
    Serial.printf("Spindle Step Count: %d, Traverse Step Count: %d, Layer Count %d Traverse Direction: %s\n", spindleStepCount, traverseStepCount, currentLayer, traverseDir ? "Forward" : "Backward");
  }
  


  if(!homed){homed = homeCycle(50, traverseDir, 5.0f, ct2,100);}
    if(launchActive){
      if(!run){
        runonce = false;
      }

      timer_core_ms(ct2, 100); // Update core timer every 100ms
      windingScreenHandler(ct2, 100); // Update the winding screen every 100ms

      if(spindle.getTurnCount() >= currentPreset.turns){
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

        if(spindle.getTurnCount() > currentPreset.turns-spin_down_turns){spindle.rampAcceleration(10, Accel_RPM, true, timer_ms, refresh_Time);}
        else{spindle.rampAcceleration(Target_RPM, Accel_RPM, false, timer_ms, refresh_Time);}
        
         // Set traverse rate based on spindle speed and wire gauge
}


void traverse_handler(unsigned long timer_ms, int refresh_Time) {
  static unsigned long lastTraverseTime = 0;
  spindle.getRate(timer_ms, refresh_Time); // Read the speed pot value and update target RPM
  if (timer_ms - lastTraverseTime < refresh_Time) return; // Limit traverse updates to every 100ms
    lastTraverseTime = timer_ms;
    currentPosition=traverse.computePosition();
    traverse.setLimits(0,10);
    traverse.setMultiplier(); // Set multiplier based on current layer count
    traverse.setTraverseRate(Current_Step_Rate, .0635);
    traverse.controlPosition(); // Control position based on step count
    currentLayer = traverse.getLayerCount(); // Update current layer count

}

