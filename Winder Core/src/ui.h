#pragma once
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "preset.h"
#include "config.h"
#include "global_vars.h"
#include "motion_HW.h"

extern TFT_eSPI tft;

enum EditingState {
  NONE,
  EDIT_NAME,
  EDIT_GAUGE,
  EDIT_WIDTH,
  EDIT_TURNS,
  EDIT_PATTERN,
  EDIT_LENGTH,
  EDIT_CENTER,
  EDIT_OVERWIND,

};

extern String editBuffer;

extern EditingState editingState;


enum MenuState {
  PRESET_SELECT,
  PRESET_EDIT,
  PRESET_WINDING
};

extern MenuState menuState;
void initUI();
void updateUI();
void drawPresetButtons();
void drawPresetEditor(int index);
void drawEditOverlay(const char* label, const String& value);
void handlePresetSelectTouch();
void handleEditScreenTouch();
void handleEditOverlayTouch();
void runCalibration();
TS_Point getMappedTouchPoint();
void drawKeyboardOverlay(const char* label, const String& value);
void handleKeyboardTouch();
void drawCursor(int x, int y);
void eraseCursor(int x, int y);
void drawPatternOverlay();
void handlePatternOverlayTouch();
void drawWindingScreen();
void handleWindingScreenTouch();
void windingScreenHandler(unsigned long timer_ms, int refresh_rate = 100);