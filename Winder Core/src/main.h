#include <Arduino.h>
#include "preset.h"
#include "ui.h"
#include <TFT_eSPI.h>
//#include "motion.h"
#include "motion_HW.h"

void timer_core_ms(unsigned long timer_ms, int refresh_Time = 100);
void ramp_handler(unsigned long timer_ms, int refresh_Time = 100);
void traverse_handler(unsigned long timer_ms, int refresh_Time = 100);