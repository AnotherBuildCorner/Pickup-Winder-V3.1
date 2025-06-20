#pragma once

#define TFT_CS    5
#define TFT_DC    16
#define TFT_RST   17

#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_MISO  12

#define TOUCH_CS  4
#define TOUCH_IRQ 15

#define SPINDLE_STEP_PIN    25
#define SPINDLE_DIR_PIN     26
#define TRAVERSE_STEP_PIN   18
#define TRAVERSE_DIR_PIN    19

#define micross 1000000

#define refreshRate 10
#define refreshmicros (micross / refreshRate)

#define steps_rev 200 //steps/rev

#define maxrpm 1500
#define speed_timer_max (60*micross/(maxrpm*steps_rev*2)) // max speed in microseconds per step

#define accel 200 //rpm/second^2
#define accel_delay_step (60*micross/(accel*steps_rev*2)) // acceleration delay in microseconds per step