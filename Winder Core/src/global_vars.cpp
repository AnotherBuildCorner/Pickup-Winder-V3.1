#include "global_vars.h"
#include "config.h"

volatile bool launchActive = false;
unsigned long steps_traversed = 0;
int turns = steps_traversed/steps_rev;
unsigned long speed = 0;
bool runflag = false;

int Current_RPM = 0;
bool run = true;

volatile int steps_remaining = 100000;
volatile int spindle_isr_mult = 100;

volatile int Layer = 0; // Current winding layer