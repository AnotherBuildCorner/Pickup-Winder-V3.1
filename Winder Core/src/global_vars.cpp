#include "global_vars.h"
#include "config.h"

volatile bool launchActive = false;
unsigned long steps_traversed = 0;
int turn_count = steps_traversed/steps_rev;
unsigned long speed = 0;
bool runflag = false;
int Current_RPM = 0;
int Target_RPM = 0;
unsigned long Current_Step_Rate = 0;
bool run = true;
int selectedPreset = -1;
volatile int steps_remaining = 100000;
volatile int spindle_isr_mult = 100;

volatile int Layer = 0; // Current winding layer

volatile bool tick = false; // Used to synchronize with the main loop
unsigned long tick_count = 0;

volatile uint32_t spindleStepCount = 0;

float Tensioner_reading = 0; // Current tensioner reading in grams