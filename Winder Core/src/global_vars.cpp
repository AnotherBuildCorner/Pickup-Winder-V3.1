#include "global_vars.h"
#include "config.h"


volatile bool launchActive = false;
unsigned long steps_traversed = 0;
int turn_count = 0;
unsigned long speed = 0;
bool runflag = false;
int Current_RPM = 0;
int Target_RPM = 0;
unsigned long Current_Step_Rate = 0;
bool run = false;
int selectedPreset = -1;
volatile int steps_remaining = 100000;
volatile int spindle_isr_mult = 100;
float currentPosition = 0.0f; // Current position of the traverse stepper
int currentLayer = 0; // Current winding layer
int totalLayers = 0; // Total number of layers in the winding
int calculated_length = 0;
int current_DCR = 0;
float current_multiplier = 0.0f;
int MAX_dcr = 0;
volatile int Layer = 0; // Current winding layer

volatile bool tick = false; // Used to synchronize with the main loop
unsigned long tick_count = 0;

volatile uint32_t spindleStepCount = 0;
volatile int traverseStepCount = 0;
volatile bool traverseDir = INIT_TRAVERSE; // false starts advancing in the forward direction
volatile bool enableTraverseISR = false; // Enable/disable traverse ISR
volatile bool enableSpindleISR = false; // Enable/disable spindle ISR
float plateZeroOffset = 10.1;
float Tensioner_reading = 0; // Current tensioner reading in grams

WinderPreset currentPreset = {
    "Default",
    1000, // Default turns
    "42n",   // Default gauge
    true, // Default spin direction
    50.0f, // Default width in mm
    0.0f,  // Default overwind percentage
    {2, 4, 6, 0}, // Default pattern
    48.0f, // Default length in mm
    1.2f,  // Default center space in mm
    2.0f,  // Default faceplate thickness in mm
    1.0f,   // Default edge error in mm
    0.000001f,
    0.0635f, // 42 AWG diameter
    17.0f, // min tension
    22.0f // max tension

};