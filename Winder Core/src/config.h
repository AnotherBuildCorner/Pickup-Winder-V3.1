
#pragma once
// main parameters
#define Start_RPM 100
#define Accel_RPM 200 //rpm
#define maxrpm 1500
#define refreshRate 10

// -------------- screen pins --------------
#define TFT_CS    5
#define TFT_DC    16
#define TFT_RST   17

#define TFT_MOSI  13
#define TFT_SCLK  14
#define TFT_MISO  12

#define TOUCH_CS  4
#define TOUCH_IRQ 15
//------------ Motion control pins ------------
#define SPINDLE_STEP_PIN    ((gpio_num_t)2)
#define SPINDLE_DIR_PIN     ((gpio_num_t)38)
#define SPINDLE_ENABLE_PIN  ((gpio_num_t)37)

#define TRAVERSE_STEP_PIN   ((gpio_num_t)40)
#define TRAVERSE_DIR_PIN    ((gpio_num_t)39)
#define TRAVERSE_ENABLE_PIN ((gpio_num_t)36)
//-------------  tensioner pins ---------------
#define TENSIONER_READ_PIN 20   
#define TENSIONER_SCK_PIN 21
// ----------  ISR parameters -----------
#define ISR_delay 10 // microseconds   

// ---------- Math params --------------
#define micross 1000000
#define refreshmicros (micross / refreshRate)
#define steps_rev 200 //steps/rev
#define accel 200/steps_rev //rpm/second^2
#define RPM_to_micros 60* micross / (steps_rev * 2) // conversion factor from RPM to microseconds per step