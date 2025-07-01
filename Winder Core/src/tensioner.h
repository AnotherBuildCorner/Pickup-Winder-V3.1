#include "global_vars.h"

#define testweight 10 //grams
#define testread -220000 // raw output

void tensioner_setup();
void tensioner_output(unsigned long timer_ms, int refresh_Time = 100); // Default to 100ms refresh