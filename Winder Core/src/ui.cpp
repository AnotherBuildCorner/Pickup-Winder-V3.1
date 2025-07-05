#include "ui.h"
#include "preset.h"
#include "global_vars.h"
#include <TMCStepper.h>
#include "motion_HW.h"

TFT_eSPI tft = TFT_eSPI();
SPIClass& sharedSPI = SPI;
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

MenuState menuState = PRESET_SELECT;

EditingState editingState = NONE;
String editBuffer = "";

int rawXmin = 350, rawXmax = 4000;
int rawYmin = 350, rawYmax = 4000;
 
const int keyW = 60;
const int keyH = 35;
const int spacing = 5;
const int baseX = 10;
const int baseY = 80;

int lastTouchX = -1;
int lastTouchY = -1;
bool cursorVisible = false;

int editValue = 1;
int patternEdit[20];  // max entries
int patternLength = 0;


const char* keys[4][4] = {
  {"1", "2", "3", "<"},
  {"4", "5", "6", "."},
  {"7", "8", "9", "+/-"},
  {"Back", "0", "OK", ""}
};


void initUI() {
  Serial.println("InitUI called");
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  //sharedSPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI);
  
  //ts.begin(sharedSPI);
  ts.begin();
 // ts.setRotation(3);  // Match screen orientation

 drawPresetButtons();
 Serial.println("InitUI exiting");
}

void drawPresetButtons() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  for (int i = 0; i < 8; i++) {
    int row = i / 2;
    int col = i % 2;
    int x = 10 + col * 150;
    int y = 10 + row * 60;

    tft.drawRect(x, y, 140, 50, TFT_WHITE);
    tft.setCursor(x + 10, y + 15);
    tft.setTextColor(TFT_WHITE);
    tft.print(presets[i].name);
  }
}

void updateUI() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.printf("Touch: x=%d y=%d z=%d\n", p.x, p.y, p.z);
  delay(100);}
  if (menuState == PRESET_SELECT && ts.touched()) {
    handlePresetSelectTouch();
  } 
  else if (menuState == PRESET_EDIT && ts.touched()) {
    if (editingState != NONE) {
      if (editingState == EDIT_NAME) {
        handleKeyboardTouch();         // QWERTY keyboard
      } 
      else if (editingState == EDIT_PATTERN) {
        handlePatternOverlayTouch();   // NEW: Pattern editor
      }
      else if (menuState == PRESET_WINDING && ts.touched()) {
        handleWindingScreenTouch();
        currentPreset = presets[selectedPreset];
      }
      else {
        handleEditOverlayTouch();      // Numeric pad (for everything else)
      }
    } else {
      handleEditScreenTouch();         // Tap field to begin editing
    }
  }
}


void handlePresetSelectTouch() {
  TS_Point p = getMappedTouchPoint();

  int tx = p.x;
  int ty = p.y;



  for (int i = 0; i < 8; i++) {
    int row = i / 2;
    int col = i % 2;
    int x = 10 + col * 150;
    int y = 10 + row * 60;

    if (tx > x && tx < x + 140 && ty > y && ty < y + 50) {
      selectedPreset = i;
      menuState = PRESET_EDIT;
      editingState = NONE;
      drawPresetEditor(i);
      delay(300);
      break;
    }
  }
}

void handleEditScreenTouch() {
  TS_Point p = getMappedTouchPoint();

  int tx = p.x;
  int ty = p.y;

  if (tx > 0 && tx < 320 && ty > 20 && ty < 45) {
    editingState = EDIT_NAME;
    drawKeyboardOverlay("Name", String(presets[selectedPreset].name));
  }
  else if (tx > 160 && tx < 320 && ty > 45 && ty < 70) {
    editingState = EDIT_GAUGE;
    drawEditOverlay("Gauge", String(presets[selectedPreset].gauge.c_str()));
  }
  else if (tx > 160 && tx < 320 && ty > 70 && ty < 95) {
    editingState = EDIT_WIDTH;
    drawEditOverlay("Width", String(presets[selectedPreset].width_mm, 2));
  }
  else if (tx > 160 && tx < 320 && ty > 95 && ty < 120) {
    editingState = EDIT_TURNS;
    drawEditOverlay("Turns", String(presets[selectedPreset].turns));
  }
  else if (tx > 160 && tx < 320 && ty > 120 && ty < 145) {
    editingState = EDIT_PATTERN;
      patternLength = 0;
  for (int i = 0; i < 20; i++) {
    if (presets[selectedPreset].pattern[i] == 0) break;
    patternEdit[patternLength++] = presets[selectedPreset].pattern[i];
  }

  editValue = 1;
    drawPatternOverlay();
  }
  else if (tx > 160 && tx < 320 && ty > 145 && ty < 170) {
    editingState = EDIT_LENGTH;
    drawEditOverlay("Length", String(presets[selectedPreset].length_mm, 2));
  }
  else if (tx > 160 && tx < 320 && ty > 170 && ty < 195) {
    editingState = EDIT_CENTER;
    drawEditOverlay("Center", String(presets[selectedPreset].center_space_mm, 2));
  }
  else if (tx > 160 && tx < 320 && ty > 195 && ty < 220) {
    editingState = EDIT_OVERWIND;
    drawEditOverlay("Overwind", String(presets[selectedPreset].overwind_percent, 1));
  }
  else if (tx > 20 && tx < 120 && ty > 215 && ty < 235) {
    menuState = PRESET_SELECT;
    drawPresetButtons();
    delay(200);
  }
  if (tx > 110 && tx < 210 && ty > 215 && ty < 240) {
  menuState = PRESET_WINDING;  // or PRESET_WINDING
  Serial.printf("Launching preset %d...\n", selectedPreset);
  drawWindingScreen();  // Optional placeholder screen
  launchActive = true;  // Set global flag to indicate winding is active
  currentPreset = presets[selectedPreset];  // Update current preset
  traverse.loadCompParameters(currentPreset);
  traverse.jogDistance(50,currentPreset.faceplate_thickness,true);
  return;
}
  else if (tx > 200 && tx < 300 && ty > 215 && ty < 235) {
    Serial.printf("Preset %d saved (placeholder)\n", selectedPreset);
    delay(200);
  }
}




void drawPresetEditor(int index) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  int xLabel = 10;
  int xValue = 160;
  int y = 20;
  int yStep = 25;

  WinderPreset &p = presets[index];

  tft.setCursor(xLabel, y);      tft.print("Name:");
  tft.setCursor(xValue, y);      tft.print(p.name);            y += yStep;

  tft.setCursor(xLabel, y);      tft.print("Gauge:");
  tft.setCursor(xValue, y);      tft.print(p.gauge.c_str());           y += yStep;

  tft.setCursor(xLabel, y);      tft.print("Width (mm):");
  tft.setCursor(xValue, y);      tft.print(p.width_mm, 2);     y += yStep;

  tft.setCursor(xLabel, y);      tft.print("Turns:");
  tft.setCursor(xValue, y);      tft.print(p.turns);           y += yStep;

  tft.setCursor(xLabel, y);      tft.print("Pattern:");
  tft.setCursor(xValue, y);
  for (int i = 0; i < min(4, (int)p.pattern.size()); i++) {
    tft.print(p.pattern[i]);
    if (i < p.pattern.size() - 1) tft.print(",");
  }
  y += yStep;

  tft.setCursor(xLabel, y);      tft.print("Length (mm):");
  tft.setCursor(xValue, y);      tft.print(p.length_mm, 2);    y += yStep;

  tft.setCursor(xLabel, y);      tft.print("Center (mm):");
  tft.setCursor(xValue, y);      tft.print(p.center_space_mm, 2); y += yStep;

  tft.setCursor(xLabel, y);      tft.print("Over/Under (%):");
  tft.setCursor(xValue, y);      tft.print(p.overwind_percent, 1); y += yStep + 10;

  // Draw buttons
  tft.fillRect(20, 215, 100, 20, TFT_DARKGREY);
  tft.setCursor(40, 218); tft.setTextColor(TFT_WHITE); tft.print("Back");

  tft.fillRect(200, 215, 100, 20, TFT_DARKGREY);
  tft.setCursor(225, 218); tft.setTextColor(TFT_WHITE); tft.print("Save");

  tft.fillRect(110, 215, 100, 25, TFT_BLUE);
tft.setTextColor(TFT_WHITE);
tft.setCursor(125, 222);
tft.print("Launch");
}

void handleEditOverlayTouch() {
  TS_Point p = getMappedTouchPoint();
  int tx = p.x;
  int ty = p.y;

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      const char* label = keys[row][col];
      if (label[0] == '\0') continue;  // faster than String()

      int x = baseX + col * (keyW + spacing);
      int y = baseY + row * (keyH + spacing);

      if (tx > x && tx < x + keyW && ty > y && ty < y + keyH) {
        // Serial.printf("Touch at row %d, col %d: %s\n", row, col, label);

        if (strcmp(label, "<") == 0) {
          if (!editBuffer.isEmpty()) editBuffer.remove(editBuffer.length() - 1);
        }
        else if (strcmp(label, "Back") == 0) {
        // 🆕 Exit without saving
        editingState = NONE;
        drawPresetEditor(selectedPreset);
        return;
        }
        else if (strcmp(label, "OK") == 0) {
          // TODO: Apply editBuffer to preset field
          editingState = NONE;
          drawPresetEditor(selectedPreset);
          return;
        }
        else if (strcmp(label, "+/-") == 0) {
          if (editBuffer.startsWith("-")) {
            editBuffer.remove(0, 1);
          } else {
            editBuffer = "-" + editBuffer;
          }
        }
        else {
          if (editBuffer.length() < 12) editBuffer += label;
        }

        // Update input field
        tft.fillRect(10, 40, 300, 30, TFT_WHITE);
        tft.setTextColor(TFT_BLACK);
        tft.setCursor(20, 45);
        tft.print(editBuffer);
        delay(150);
        return;
      }
    }
  }

  // Missed everything
  Serial.printf("Missed: tx=%d ty=%d\n", tx, ty);
}






void drawEditOverlay(const char* label, const String& value) {
  tft.fillScreen(TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  // Header
  tft.setCursor(10, 10);
  tft.print("Edit: ");
  tft.print(label);

  // Value field
  tft.fillRect(10, 40, 300, 30, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 45);
  tft.print(value);
  editBuffer = value;

  // Define keypad layout
  const char* keys[4][4] = {
    { "1", "2", "3", "<"   },
    { "4", "5", "6", "."   },
    { "7", "8", "9", "+/-" },
    { "Back", "0", "OK", ""   }
  };

  const int keyW = 60;
  const int keyH = 35;
  const int spacing = 5;
  const int baseX = 10;
  const int baseY = 80;

  // Draw keys
  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 4; col++) {
      const char* label = keys[row][col];
      if (label[0] == '\0') continue;  // skip empty

      int x = baseX + col * (keyW + spacing);
      int y = baseY + row * (keyH + spacing);

      uint16_t bgColor = TFT_BLUE;

      if (strcmp(label, "Back") == 0)      bgColor = TFT_RED;
      else if (strcmp(label, "OK") == 0) bgColor = TFT_GREEN;
      else if (strcmp(label, "<") == 0)  bgColor = TFT_ORANGE;
      else if (strcmp(label, "+/-") == 0) bgColor = TFT_DARKCYAN;

      tft.fillRect(x, y, keyW, keyH, bgColor);
      tft.setTextColor(TFT_WHITE);
      int labelX = (strlen(label) == 1) ? x + 22 : x + 14;
      tft.setCursor(labelX, y + 10);
      tft.print(label);
    }
  }
}

void runCalibration() {
  TS_Point p;
  int xRaw[3], yRaw[3];
  int targetX[3] = {20, 300, 160};
  int targetY[3] = {20, 220, 120};

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  for (int i = 0; i < 3; i++) {
    // Draw crosshair
    tft.fillScreen(TFT_BLACK);
    tft.drawLine(targetX[i] - 5, targetY[i], targetX[i] + 5, targetY[i], TFT_RED);
    tft.drawLine(targetX[i], targetY[i] - 5, targetX[i], targetY[i] + 5, TFT_RED);
    tft.setCursor(10, 10);
    tft.print("          Touch target...");

    // Wait for touch and collect raw data
    while (!ts.touched()) delay(10);
    delay(100); // debounce

    p = ts.getPoint();
    xRaw[i] = p.x;
    yRaw[i] = p.y;

    while (ts.touched()) delay(10); // wait for release
    delay(300); // debounce
  }

  // Calculate min/max
  rawXmin = min(xRaw[0], xRaw[1]);
  rawXmax = max(xRaw[0], xRaw[1]);
  rawYmin = min(yRaw[0], yRaw[1]);
  rawYmax = max(yRaw[0], yRaw[1]);

  // Feedback
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.print("Cal complete:");
  tft.setCursor(10, 30);
  tft.printf("X: %d - %d", rawXmin, rawXmax);
  tft.setCursor(10, 50);
  tft.printf("Y: %d - %d", rawYmin, rawYmax);

  delay(1000);
}

TS_Point getMappedTouchPoint() {
  TS_Point raw = ts.getPoint();
  TS_Point mapped;

  mapped.x = map(raw.x, rawXmin, rawXmax, tft.width(), 0);      // flip X
  mapped.y = map(raw.y, rawYmin, rawYmax, 0, tft.height());     // normal Y

  // Optional: match against screen bounds
  mapped.x = constrain(mapped.x, 0, tft.width() - 1);
  mapped.y = constrain(mapped.y, 0, tft.height() - 1);

  // --- Cursor Logic ---
  if (cursorVisible) {
    eraseCursor(lastTouchX, lastTouchY);
  }

  lastTouchX = mapped.x;
  lastTouchY = mapped.y;
  cursorVisible = true;
  drawCursor(mapped.x, mapped.y);

  return mapped;
}


void drawKeyboardOverlay(const char* label, const String& value) {
  tft.fillScreen(TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);

  tft.setCursor(10, 10);
  tft.print("Edit: ");
  tft.print(label);

  // Value box
  tft.fillRect(10, 40, 300, 30, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 45);
  tft.print(value);
  editBuffer = value;

  // Keyboard layout
  const char* row1 = "QWERTYUIOP";
  const char* row2 = "ASDFGHJKL";
  const char* row3 = "ZXCVBNM";
  int keyW = 28, keyH = 30, spacing = 4;

  // Row 1
  for (int i = 0; i < 10; i++) {
    int x = 4 + i * (keyW + spacing);
    int y = 80;
    tft.fillRect(x, y, keyW, keyH, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(x + 8, y + 8);
    tft.print(row1[i]);
  }

  // Row 2
  for (int i = 0; i < 9; i++) {
    int x = 18 + i * (keyW + spacing);  // slight offset
    int y = 80 + keyH + spacing;
    tft.fillRect(x, y, keyW, keyH, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(x + 8, y + 8);
    tft.print(row2[i]);
  }

  // Row 3 + Backspace
  for (int i = 0; i < 7; i++) {
    int x = 46 + i * (keyW + spacing);  // further offset
    int y = 80 + 2 * (keyH + spacing);
    tft.fillRect(x, y, keyW, keyH, TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(x + 8, y + 8);
    tft.print(row3[i]);
  }
  // Backspace
  int bsX = 46 + 7 * (keyW + spacing);
  int bsY = 80 + 2 * (keyH + spacing);
  tft.fillRect(bsX, bsY, keyW * 1.5, keyH, TFT_RED);
  tft.setCursor(bsX + 10, bsY + 8);
  tft.setTextColor(TFT_WHITE);
  tft.print("<-");

  // Row 4: SPACE + OK
  int spX = 20;
  int spY = bsY + keyH + spacing;
  tft.fillRect(spX, spY, keyW * 5, keyH, TFT_BLUE);
  tft.setCursor(spX + 40, spY + 8);
  tft.setTextColor(TFT_WHITE);
  tft.print("SPACE");

  int okX = spX + keyW * 5 + spacing + 10;
  tft.fillRect(okX, spY, keyW * 2, keyH, TFT_GREEN);
  tft.setCursor(okX + 10, spY + 8);
  tft.setTextColor(TFT_WHITE);
  tft.print("OK");
}

void handleKeyboardTouch() {
  TS_Point p = getMappedTouchPoint();
  int tx = p.x, ty = p.y;

  const int keyW = 28, keyH = 30, spacing = 4;
  const char* row1 = "QWERTYUIOP";
  const char* row2 = "ASDFGHJKL";
  const char* row3 = "ZXCVBNM";

  // Moved up to avoid jump-over-init error
  int x, y;
  int bsX = 46 + 7 * (keyW + spacing);
  int bsY = 80 + 2 * (keyH + spacing);
  int spX = 20;
  int spY = bsY + keyH + spacing;
  int okX = spX + keyW * 5 + spacing + 10;

  // Row 1
  for (int i = 0; i < 10; i++) {
    x = 4 + i * (keyW + spacing);
    y = 80;
    if (tx > x && tx < x + keyW && ty > y && ty < y + keyH) {
      editBuffer += row1[i];
      goto updateDisplay;
    }
  }

  // Row 2
  for (int i = 0; i < 9; i++) {
    x = 18 + i * (keyW + spacing);
    y = 80 + keyH + spacing;
    if (tx > x && tx < x + keyW && ty > y && ty < y + keyH) {
      editBuffer += row2[i];
      goto updateDisplay;
    }
  }

  // Row 3
  for (int i = 0; i < 7; i++) {
    x = 46 + i * (keyW + spacing);
    y = 80 + 2 * (keyH + spacing);
    if (tx > x && tx < x + keyW && ty > y && ty < y + keyH) {
      editBuffer += row3[i];
      goto updateDisplay;
    }
  }

  // Backspace
  if (tx > bsX && tx < bsX + keyW * 1.5 && ty > bsY && ty < bsY + keyH) {
    if (!editBuffer.isEmpty()) editBuffer.remove(editBuffer.length() - 1);
    goto updateDisplay;
  }

  // SPACE
  if (tx > spX && tx < spX + keyW * 5 && ty > spY && ty < spY + keyH) {
    editBuffer += ' ';
    goto updateDisplay;
  }

  // OK
  if (tx > okX && tx < okX + keyW * 2 && ty > spY && ty < spY + keyH) {
    if (editingState == EDIT_NAME) {
      presets[selectedPreset].name = editBuffer;
    }
    editingState = NONE;
    drawPresetEditor(selectedPreset);
    return;
  }

updateDisplay:
  tft.fillRect(10, 40, 300, 30, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(20, 45);
  tft.print(editBuffer);
  delay(150);
}

void drawCursor(int x, int y) {
  const int r = 5;
  tft.drawLine(x - r, y, x + r, y, TFT_YELLOW);  // horizontal line
  tft.drawLine(x, y - r, x, y + r, TFT_YELLOW);  // vertical line
}

void eraseCursor(int x, int y) {
  const int r = 5;
  tft.drawLine(x - r, y, x + r, y, TFT_DARKGREY);  // match background
  tft.drawLine(x, y - r, x, y + r, TFT_DARKGREY);
}


void drawPatternOverlay() {
  tft.fillScreen(TFT_DARKGREY);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  // Header
  tft.setCursor(10, 10);
  tft.print("Edit Pattern");

  // Pattern display
  tft.setCursor(10, 40);
  tft.print("Pattern: ");
  for (int i = 0; i < patternLength; i++) {
    tft.print(patternEdit[i]);
    tft.print(" ");
  }

  // Current edit value
  tft.fillRect(10, 70, 300, 40, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(130, 80);
  tft.setTextSize(3);
  tft.print(editValue);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  // Buttons
  const int keyW = 80;
  const int keyH = 40;
  const int spacing = 10;
  const int baseX = 10;
  const int baseY = 130;

  const char* labels[2][3] = {
    { "^", "Add", "Back" },
    { "V", "<",   "OK"  }
  };

  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 3; col++) {
      int x = baseX + col * (keyW + spacing);
      int y = baseY + row * (keyH + spacing);
      tft.fillRect(x, y, keyW, keyH, TFT_BLUE);
      tft.setCursor(x + 25, y + 10);
      tft.print(labels[row][col]);
    }
  }
}

void handlePatternOverlayTouch() {
  TS_Point p = getMappedTouchPoint();
  int tx = p.x;
  int ty = p.y;

  const int keyW = 80;
  const int keyH = 40;
  const int spacing = 10;
  const int baseX = 10;
  const int baseY = 130;

  const char* labels[2][3] = {
    { "▲", "Add", "CLR" },
    { "▼", "←",   "OK"  }
  };

  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < 3; col++) {
      int x = baseX + col * (keyW + spacing);
      int y = baseY + row * (keyH + spacing);
      if (tx > x && tx < x + keyW && ty > y && ty < y + keyH) {
        const char* label = labels[row][col];

        if (strcmp(label, "▲") == 0) {
          editValue = (editValue + 1) % 51;
          if (editValue == 0) editValue = 1;
        } 
        else if (strcmp(label, "▼") == 0) {
          editValue--;
          if (editValue <= 0) editValue = 50;
        } 
        else if (strcmp(label, "Add") == 0 && patternLength < 20) {
          patternEdit[patternLength++] = editValue;
        } 
        else if (strcmp(label, "←") == 0 && patternLength > 0) {
          patternLength--;
        } 
        else if (strcmp(label, "CLR") == 0) {
        // 🆕 Exit without saving
        editingState = NONE;
        drawPresetEditor(selectedPreset);
        return;
        }
        else if (strcmp(label, "OK") == 0) {
          // Apply to preset
          for (int i = 0; i < 20; i++) {
            presets[selectedPreset].pattern[i] = (i < patternLength) ? patternEdit[i] : 0;
          }
          editingState = NONE;
          drawPresetEditor(selectedPreset);
          return;
        }

        drawPatternOverlay();  // re-render UI
        return;
      }
    }
  }
}

void drawTestScreen() {
  int screenstart = 40;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(40, screenstart);
  tft.print("Winding Started...");
  tft.setCursor(40, screenstart + 20);
  tft.print("turns count: ");
  tft.print(turn_count);
  tft.setCursor(40, screenstart + 40);
  tft.print("Current RPM: ");
  tft.print(Current_RPM);
  tft.setCursor(40, screenstart + 60);
  tft.print("Target RPM: ");
  tft.print(Target_RPM);
  tft.setCursor(40, screenstart + 80);
  tft.print("stepcount: ");
  tft.print(spindleStepCount);
  tft.setCursor(40, screenstart + 100);
  tft.print("tension: ");
  tft.print(Tensioner_reading);
}

void drawWindingScreen() {
  int screenstart = 20;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);

  // Title
  tft.setCursor(60, screenstart);
  tft.print("Winding Control");

  // Stats
  tft.setTextColor(TFT_WHITE);
  int y = screenstart + 30;
  int xLabel = 20, xValue = 180, yStep = 22;

  // RPM: current / target
  tft.setCursor(xLabel, y);      
  tft.print("RPM:");      
  tft.setCursor(xValue, y); 
  tft.printf("%d / %d", Current_RPM, Target_RPM); 
  y += yStep;

  // Turns: current / target
  tft.setCursor(xLabel, y);      
  tft.print("Turns:");    
  tft.setCursor(xValue, y); 
  tft.printf("%d / %d", turn_count, currentPreset.turns); 
  y += yStep;

  tft.setCursor(xLabel, y);      
  tft.print("Scatter Mult:");     
  tft.setCursor(xValue, y); 
  tft.print(traverse.getMultiplier()); 
  y += yStep;

  tft.setCursor(xLabel, y);      
  tft.print("Tension:");          
  tft.setCursor(xValue, y); 
  tft.print(Tensioner_reading); 
  y += yStep;

  tft.setCursor(xLabel, y);      
  tft.print("Linear Pos:");       
  tft.setCursor(xValue, y); 
  tft.print(currentPosition); 
  y += yStep;

  tft.setCursor(xLabel, y);      
  tft.print("Layer:");            
  tft.setCursor(xValue, y); 
  tft.printf("%d/%d", currentLayer, totalLayers);

    int buttonY = 200; // was 220

  // Back Button (left side, where Stop was)
  tft.fillRect(20, buttonY, 80, 35, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(40, buttonY + 10);
  tft.print("Back");

  // Jog Arrows (Left/Right)
  tft.fillRect(120, buttonY, 35, 35, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(130, buttonY + 12);
  tft.print("<");

  tft.fillRect(165, buttonY, 35, 35, TFT_BLUE);
  tft.setCursor(180, buttonY + 12);
  tft.print(">");

  // Start/Stop Button (right side, opposite Back)
  if (run) {
    tft.fillRect(220, buttonY, 80, 35, TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(245, buttonY + 10);
    tft.print("Stop");
  } else {
    tft.fillRect(220, buttonY, 80, 35, TFT_GREEN);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(245, buttonY + 10);
    tft.print("Start");
  }


  // Restore default text color
  tft.setTextColor(TFT_WHITE);
}

void handleWindingScreenTouch() {
  TS_Point p = getMappedTouchPoint();
  int tx = p.x;
  int ty = p.y;

  int buttonY = 200; // was 220

  // Back Button (left)
  if (tx > 20 && tx < 100 && ty > buttonY && ty < buttonY + 35) {
    Serial.println("Back pressed");
    menuState = PRESET_EDIT;
    drawPresetEditor(selectedPreset);
    run = false;
    return;
  }

  // Left Jog Arrow
  if (tx > 120 && tx < 155 && ty > buttonY && ty < buttonY + 35) {
    Serial.println("Jog Left");
    traverse.jogDistance(100,0.1,false,true);
    
    return;
  }

  // Right Jog Arrow
  if (tx > 165 && tx < 200 && ty > buttonY && ty < buttonY + 35) {
    Serial.println("Jog Right");
    traverse.jogDistance(100,0.1,true,true);
    return;
  }

  // Start/Stop Button (right)
  if (tx > 220 && tx < 300 && ty > buttonY && ty < buttonY + 35) {
    run = !run;
    Serial.printf("Winding %s\n", run ? "started" : "stopped");
    return;
  }
}

void windingScreenHandler(unsigned long timer_ms, int refresh_rate) {
  static unsigned long lastUpdate = 0;
  static unsigned long lastTouchTime = 0;
  if (timer_ms - lastUpdate >= refresh_rate) {
    lastUpdate = timer_ms;
    drawWindingScreen();
  }

  if (ts.touched()) {
    if(timer_ms - lastTouchTime > 200) {
      lastTouchTime = timer_ms;
      handleWindingScreenTouch();
    }
    
  }

}