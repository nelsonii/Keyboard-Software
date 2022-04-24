// 7MM_Keyboard_4in1Cursor_v1C
// 2022-03-25-F

//=================================================================================

#include <EEPROM.h>
#include "Joystick.h" // PC Only -- Not XBOX AC
#include "Keyboard.h"
#include "Mouse.h"
#include <Adafruit_NeoPixel.h>

//=================================================================================

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
                   JOYSTICK_TYPE_JOYSTICK, 
                   0, 0,
                   true, true, false, false, false, false,
                   false, false, false, false, false);

// constructor: use non-default id, it's a joystick, no buttons,
//              there are no hat buttons, there is an X and Y analog
//              and no other controls.

// WARNING: This will NOT work with the XBOX Adaptive Controller
//          Adding Keyboard will cause the XBOX AC to ignore the joystick.

//=================================================================================

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, 10, NEO_GRB + NEO_KHZ800);

uint32_t red = pixels.Color(16,0,0);
uint32_t green = pixels.Color(0,13,0);
uint32_t blue = pixels.Color(0,0,16);
uint32_t yellow = pixels.Color(13,13,0);
uint32_t aqua = pixels.Color(0,6,6);
uint32_t black = pixels.Color(0,0,0);
uint32_t white = pixels.Color(10,10,10);

uint32_t dimred = pixels.Color(4,0,0);
uint32_t dimgreen = pixels.Color(0,3,0);
uint32_t dimblue = pixels.Color(0,0,4);
uint32_t dimyellow = pixels.Color(3,3,0);

//=================================================================================

const boolean debug = true; // if true, output info to serial monitor or plotter.
const boolean debugVerbose = true; // if true, output more info to serial monitor.
const boolean debugPlot = false; // if true, output data for plotter/graph viewing.

//=================================================================================

// Keypad pins
#define SW_UP 4
#define SW_DN 5  
#define SW_LT 6
#define SW_RT 7

//=================================================================================

// values from switches
bool rawLT, rawRT, rawUP, rawDN;

//=================================================================================

// invert if needed
const bool invHorz = false;
const bool invVert = false;

//=================================================================================

const int joyMin = 0;
const int joyMax = 1023;

//=================================================================================

// set the default min/max values.
// Adjust values based on what you actually get from the stick being used.

int minHorz=0;
int centeredHorz=512;
int maxHorz=1023;

int minVert=0;
int centeredVert=512;
int maxVert=1023;

int adjHorz = 512;
int adjVert = 512;
int mapHorz;
int mapVert;


//=================================================================================

//Mode of the device

int mode = 0;
#define WASD 1
#define CURS 2
#define MOUS 3
#define JOYS 4

//=================================================================================

void setup() {

  if (debug) { 
    Serial.begin(115200);
    Serial.println("FourInOneCursorKeyboard");
  }

  // Set U/D/L/R (from keypad) to input
  pinMode(SW_UP, INPUT_PULLUP);
  pinMode(SW_DN, INPUT_PULLUP);
  pinMode(SW_LT, INPUT_PULLUP);
  pinMode(SW_RT, INPUT_PULLUP);

  //Turn off the TX/RX red LEDs
  pinMode(LED_BUILTIN_TX,INPUT);
  pinMode(LED_BUILTIN_RX,INPUT);

  //Get the default/last mode.
  mode=getDefaultSetting();

  Keyboard.begin();
  Mouse.begin(); 
  Joystick.begin(false);
  Joystick.setXAxisRange(joyMin, joyMax); Joystick.setXAxis(0);
  Joystick.setYAxisRange(joyMin, joyMax); Joystick.setYAxis(0);
  Joystick.sendState();


  // Startup the NeoPixel
  pixels.begin(); pixels.clear(); pixels.show();

}//setup


//=================================================================================


void loop() {

  // Read values from input pins.
  rawUP = digitalRead(SW_UP);
  rawDN = digitalRead(SW_DN);
  rawLT = digitalRead(SW_LT);
  rawRT = digitalRead(SW_RT);

  //Check to see if user is changing the mode
  if (!rawUP && !rawDN && !rawLT && !rawRT) { mode=checkForSettingChange(); }

  if (mode==WASD) { processWASD(); }
  if (mode==CURS) { processCursor(); }
  if (mode==MOUS) { processMouse(); }
  if (mode==JOYS) { processJoystick(); }

  pixelKeypress();
  
  if (debug) {serialDebug();}

}//loop


//=================================================================================

void processWASD() {

  // Note that the digital input pins are set as PULLUP
  // so we look for the inverted (zero) boolean value!!

  if (!rawUP) { Keyboard.press('w'); }
  else if (!rawLT) { Keyboard.press('a'); }
  else if (!rawDN) { Keyboard.press('s'); }
  else if (!rawRT) { Keyboard.press('d'); }
  else { Keyboard.releaseAll(); }
  
}

//=================================================================================

void processCursor() {

  // Note that the digital input pins are set as PULLUP
  // so we look for the inverted (zero) boolean value!!

  if (!rawUP) { Keyboard.press(KEY_UP_ARROW); }
  else if (!rawDN) { Keyboard.press(KEY_DOWN_ARROW); }
  else if (!rawLT) { Keyboard.press(KEY_LEFT_ARROW); }
  else if (!rawRT) { Keyboard.press(KEY_RIGHT_ARROW); }
  else { Keyboard.releaseAll(); }
  
}

//=================================================================================

void processMouse() {

  // Note that the digital input pins are set as PULLUP
  // so we look for the inverted (zero) boolean value!!

  if (!rawUP) { Mouse.move(0, -1); }
  if (!rawDN) { Mouse.move(0, +1); }
  if (!rawLT) { Mouse.move(-1, 0); }
  if (!rawRT) { Mouse.move(+1, 0); }
  
}

//=================================================================================

void processJoystick() {

  int stepsPerLoop = 2;
  
  //vertical - up/down
  if (!rawUP) { if (adjVert>minVert) { adjVert=adjVert-stepsPerLoop; } else { adjVert=minVert; } } 
  if (!rawDN) { if (adjVert<maxVert) { adjVert=adjVert+stepsPerLoop; } else { adjVert=maxVert; } }
  if (rawUP && rawDN) { adjVert=centeredVert; }
  
  if (!rawLT) { if (adjHorz>minHorz) {adjHorz=adjHorz-stepsPerLoop;} else {adjHorz=minHorz;} } 
  if (!rawRT) { if (adjHorz<maxHorz) {adjHorz=adjHorz+stepsPerLoop;} else {adjHorz=maxHorz;} } 
  if (rawLT && rawRT) { adjHorz=centeredHorz; }

  // Map values to a range the XAC likes
  mapVert = map(adjVert, minVert, maxVert, joyMin, joyMax);
  mapHorz = map(adjHorz, minHorz, maxHorz, joyMin, joyMax);

  // Invert value if requested (if "up" should go "down" or "left" to "right")
  if (invVert) {mapVert = -mapVert;}
  if (invHorz) {mapHorz = -mapHorz;}

  // Send the values to the joystick object
  Joystick.setYAxis(mapVert);
  Joystick.setXAxis(mapHorz);

  // Send updated joystick state to HID upstream
  Joystick.sendState();

}


//=================================================================================

void pixelKeypress() {

  if (!rawUP || !rawDN || !rawLT || !rawRT) {
    // A button was pressed, show bright color associated with current mode
    if (mode==JOYS) { pixels.clear(); pixels.setPixelColor(0, red); pixels.show(); }
    else if (mode==MOUS) { pixels.clear(); pixels.setPixelColor(0, green); pixels.show(); }
    else if (mode==WASD) { pixels.clear(); pixels.setPixelColor(0, blue); pixels.show(); }
    else if (mode==CURS) { pixels.clear(); pixels.setPixelColor(0, yellow); pixels.show(); }
    else {pixels.clear(); pixels.setPixelColor(0, black); pixels.show(); }
  }
  else {
    // No buttons currently pressed. Show dim color associated with current mode
    if (mode==JOYS) { pixels.clear(); pixels.setPixelColor(0, dimred); pixels.show(); }
    else if (mode==MOUS) { pixels.clear(); pixels.setPixelColor(0, dimgreen); pixels.show(); }
    else if (mode==WASD) { pixels.clear(); pixels.setPixelColor(0, dimblue); pixels.show(); }
    else if (mode==CURS) { pixels.clear(); pixels.setPixelColor(0, dimyellow); pixels.show(); }
    else {pixels.clear(); pixels.setPixelColor(0, black); pixels.show(); }
  }

}

//=================================================================================

int getDefaultSetting() {

  int eeAddress = 0; // Address zero
  byte eeValue = EEPROM.read(eeAddress); 

  //If invalid value (never initialized) set to MOUSE mode and update EEPROM
  if (eeValue < 1 || eeValue > 4) {
    eeValue = MOUS;
    EEPROM.update(eeAddress, eeValue); 
    }
  
  return (int) eeValue;
 
}

//=================================================================================

int checkForSettingChange() {

  int selectedMode = mode;
  int startupDelay = 5000; // 5 seconds

  //Quit sending keys during setting change
  if (mode==WASD || mode==CURS) { Keyboard.releaseAll(); }

  // Wait for user to release all buttons
  while(!digitalRead(SW_UP) || !digitalRead(SW_DN) || !digitalRead(SW_LT) || !digitalRead(SW_RT)) { 
    pixels.clear(); pixels.setPixelColor(0, black); pixels.show(); delay(100);    
    pixels.clear(); pixels.setPixelColor(0, white); pixels.show(); delay(100);    
  }

  long startupTime = millis();
  while((millis()-startupTime) <= startupDelay) {
    if (!digitalRead(SW_UP)) { selectedMode = JOYS; break; }
    if (!digitalRead(SW_DN)) { selectedMode = MOUS; break; }
    if (!digitalRead(SW_LT)) { selectedMode = WASD; break; }
    if (!digitalRead(SW_RT)) { selectedMode = CURS; break; }
  }

  pixels.clear(); pixels.setPixelColor(0, black); pixels.show();    

  // Wait for user to release all buttons
  while(!digitalRead(SW_UP) || !digitalRead(SW_DN) || !digitalRead(SW_LT) || !digitalRead(SW_RT)) { }

  // Update eeprom if mode changed
  if (selectedMode!=mode) {
    int eeAddress = 0; // Address zero
    EEPROM.update(eeAddress, selectedMode);
  }

  delay(250);
  return selectedMode;  
}

//=================================================================================

void serialDebug() {
  
    if (!debugPlot && debugVerbose) {
        Serial.print("mode: ");
        Serial.print(mode);
        Serial.print(" ");
        Serial.print("U/D/L/R: ");
        Serial.print(rawUP);
        Serial.print(" ");
        Serial.print(rawDN);
        Serial.print(" ");
        Serial.print(rawLT);
        Serial.print(" ");
        Serial.print(rawRT);
        Serial.print(" ");
      }//!debugPlot && debugVerbose

      if (!debugPlot) {
        Serial.print("Adj V/H: ");
        Serial.print(adjVert);
        Serial.print(" ");
        Serial.print(adjHorz);
        Serial.print(" ");
    }//!debugPlot

    if (!debugPlot) {Serial.print("Map V/H: ");}
    
    Serial.print(mapVert);
    if (invVert && !debugPlot) { Serial.print("i"); } // i indicates value was inverted
    Serial.print(" ");
    Serial.print(mapHorz);
    if (invHorz && !debugPlot) { Serial.print("i"); } // i indicates value was inverted 
    Serial.print(" ");
    
    Serial.println();

}//serialDebug

//=================================================================================
