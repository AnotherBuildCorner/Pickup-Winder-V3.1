
#pragma once
// main parameters
#define Start_RPM 100
#define Accel_RPM 200 //rpm
#define maxrpm 1000
#define refreshRate 10
#define spin_down_turns 30
#define Lead_Screw_Pitch 2 //mm

// -------------- screen pins --------------
#define TOUCH_IRQ 4
#define TOUCH_CS  7


#define TFT_CS    18
#define TFT_DC    16
#define TFT_RST   17

#define TFT_MISO  5  //T_DO
#define TFT_SCLK  15  //T_CLK
#define TFT_MOSI  6  //T_DIN

#define SD_CS 10
#define SD_MOSI 11
#define SD_MISO 13
#define SD_SCK 12


#define SPEED_POT 14  //3
#define speedPotAveraging 10
#define speedPotChunk 20
//------------ Motion control pins ------------
#define SPINDLE_STEP_PIN    ((gpio_num_t)2)
#define SPINDLE_DIR_PIN     ((gpio_num_t)42)
#define SPINDLE_ENABLE_PIN  ((gpio_num_t)1)  // 41  1
#define SPINDLE_READBACK_PIN  ((gpio_num_t)41) // 40   45

#define SPINDLE_RX_PIN      ((gpio_num_t)40)
#define SPINDLE_TX_PIN      ((gpio_num_t)39)  //39 46
#define SPINDLE_MICROSTEPS 2


#define TRAVERSE_STEP_PIN   ((gpio_num_t)38)
#define TRAVERSE_DIR_PIN    ((gpio_num_t)37)
#define TRAVERSE_ENABLE_PIN ((gpio_num_t)36)
#define TRAVERSE_READBACK_PIN ((gpio_num_t)35)

//#define TRAVERSE_RX_PIN      ((gpio_num_t)48)  // not in use
//#define TRAVERSE_TX_PIN      ((gpio_num_t)47)

#define TRAVERSE_MICROSTEPS 16
#define MAX_TRAVERSE_MULTIPLIER 10 // Maximum multiplier for traverse step rate

#define DIAG_PIN ((gpio_num_t)21)  //13
//-------------  tensioner pins ---------------
#define TENSIONER_READ_PIN 3   
#define TENSIONER_SCK_PIN 8
// ----------  ISR parameters -----------
#define ISR_delay 10 // microseconds   

// ---------- Math params --------------
#define micross 1000000
#define refreshmicros (micross / refreshRate)
#define steps_rev 200 //steps/rev
#define accel 200/steps_rev //rpm/second^2
#define RPM_to_micros 60* micross / (steps_rev * 2) // conversion factor from RPM to microseconds per step

