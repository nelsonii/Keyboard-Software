// 7MM_ChordKeys_QTPy_SAMD21_v1A
// 2022-08-18-A

const long buildDate = 2022081801;

// Board: Adafruit QT PY (SAMD21 or similar)
// Optimize: Fast (-O2) (Fast or Faster Recommended)
// USB Stack: TinyUSB (TinyUSB for HID work).
// Debug: Off
// Port: COMxx (Adafruit QT PY (SAMD21 or similar)

// This sketch is only valid on boards which have native USB support
// and compatibility with Adafruit TinyUSB library. 
// For example SAMD21, SAMD51, nRF52840.

//=================================================================================

#include <Adafruit_TinyUSB.h> // HID support
#include <Adafruit_NeoPixel.h> // on-board RGB LED
#include <AceButton.h> // button handler
using namespace ace_button;
#include <MemoryFree.h> // available memory

//=================================================================================

#define SERIAL_BAUDRATE     115200
#define SERIAL_TIMEOUT      1000
#define HEARTBEAT           5000 // production value will be higher (60000 - one min)

//=================================================================================

const int key1Pin = A0;
const int key2Pin = A1;
const int key3Pin = A2;
const int key4Pin = A3;
const int buzzPin = SCK;

//=================================================================================

AceButton key1(key1Pin);
AceButton key2(key2Pin);
AceButton key3(key3Pin);
AceButton key4(key4Pin);

// Forward reference to prevent Arduino compiler becoming confused.
void handleEvent(AceButton*, uint8_t, uint8_t);

//=================================================================================

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, PIN_NEOPIXEL);

uint32_t redLow = pixels.Color(64,0,0);
uint32_t redHigh = pixels.Color(255,0,0);
uint32_t greenLow = pixels.Color(0,64,0);
uint32_t greenHigh = pixels.Color(0,255,0);
uint32_t blueLow = pixels.Color(0,0,64);
uint32_t blueHigh = pixels.Color(0,0,255);
uint32_t pinkLow = pixels.Color(64,13,30);
uint32_t pinkHigh = pixels.Color(255,51,119);
uint32_t yellowLow = pixels.Color(64,43,0);
uint32_t yellowHigh = pixels.Color(255,170,0);
uint32_t whiteLow = pixels.Color(64,64,64);
uint32_t whiteHigh = pixels.Color(255,255,255);
uint32_t violetLow = pixels.Color(38,0,64);
uint32_t violetHigh = pixels.Color(153, 0, 255);
uint32_t cyanLow = pixels.Color(0,64,64);
uint32_t cyanHigh = pixels.Color(0,255,255);
uint32_t black = pixels.Color(0,0,0);

//=================================================================================

  uint8_t const desc_hid_report[] = { TUD_HID_REPORT_DESC_KEYBOARD() };
  Adafruit_USBD_HID usb_hid(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_KEYBOARD, 2, false);

//=================================================================================

void setup() {

  // COLOR STATUS CODES
  // Cyan -- serial startup and connection (or timeout)
  // Pink -- TinyUSB mounting (blocking)
  // Yellow -- ACE button configuration
  // Blue -- Test buzzer
  // Green -- Setup complete

  //Pin configs
  pinMode(key1Pin, INPUT_PULLUP);
  pinMode(key2Pin, INPUT_PULLUP);
  pinMode(key3Pin, INPUT_PULLUP);
  pinMode(key4Pin, INPUT_PULLUP);
  pinMode(buzzPin, OUTPUT);

  //Startup the onboard NeoPixel
  pixels.begin(); 

  //Startup serial monitoring (optional)
  pixels.clear(); pixels.setPixelColor(0, cyanHigh); pixels.show();
  Serial.begin(SERIAL_BAUDRATE);
  
  //Wait for the serial monitor to be opened, or timeout after x seconds
  unsigned long serialTimeout = millis(); 
  while (!Serial && (millis() - serialTimeout <= SERIAL_TIMEOUT)) {} 
  delay(200);
  Serial.println();
  Serial.println("----------------------------------------------------------");
  Serial.println();
  Serial.print("7MM_ChordKeys_QTPy_SAMD21_v1A "); Serial.println(buildDate);
  Serial.println();


  // Startup TinyUSB for Keyboard (can also handle mouse, joystick, etc)
  // IMPORTANT: This will loop until device is mounted.
  //            So, if you see a PINK light, it's powered, but not mounted.
  //            When the LED goes yellow, it's mounted.
  pixels.clear(); pixels.setPixelColor(0, pinkLow); pixels.show();
  usb_hid.begin();
  while( !TinyUSBDevice.mounted() ) delay(10);

  // Configure the ButtonConfig with the event handler, and enable all higher level events.
  pixels.clear(); pixels.setPixelColor(0,yellowLow); pixels.show();
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);

  //Buzz to indicate adjustments complete
  pixels.clear(); pixels.setPixelColor(0, blueLow); pixels.show();
  tone(buzzPin, 800); delay(500); noTone(buzzPin); delay(500);

  Serial.println("Setup phase Complete.");
  pixels.clear(); pixels.setPixelColor(0, greenLow); pixels.show();
  Serial.println("\tStarting main Loop...");


}//setup

//=================================================================================


void loop() {

  //Check the keys
  key1.check(); key2.check(); key3.check(); key4.check();

  //Display available memory on a regular basis
  HeartBeat();

}//loop

//=================================================================================


void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // The event handler for all keys/buttons

  // Print out a message for all events, for all buttons.
  Serial.print(F("handleEvent(): pin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  switch (eventType) {
    case AceButton::kEventClicked:
      if (button->getPin() == key1Pin) {
        Serial.println(F("Button 1 clicked!"));
      } else if (button->getPin() == key2Pin) {
        Serial.println(F("Button 2 clicked!"));
      } else if (button->getPin() == key3Pin) {
        Serial.println(F("Button 3 clicked!"));
      } else if (button->getPin() == key4Pin) {
        Serial.println(F("Button 4 clicked!"));
      }      
      break;
    }

}


//=================================================================================

void alarmTone(int level) {

  switch (level) {
    case 1:
      tone(buzzPin, 1200); delay(200); noTone(buzzPin);
      break;
    case 2:
      tone(buzzPin, 1000); delay(300); noTone(buzzPin); 
      tone(buzzPin, 800); delay(300); noTone(buzzPin); 
      break;
    case 3:
      tone(buzzPin, 1000); delay(200); noTone(buzzPin); 
      tone(buzzPin, 800); delay(200); noTone(buzzPin); 
      tone(buzzPin, 600); delay(200); noTone(buzzPin); 
      tone(buzzPin, 800); delay(200); noTone(buzzPin); 
      break;
    default:
      break;
  }//switch
  
}//alarmTone

//=================================================================================

void HeartBeat() 
{
  // Show free memory every (HEARTBEAT) seconds 
  static unsigned long last = millis();
  if (millis() - last > HEARTBEAT) {
    last = millis();
    Serial.printf("CK %d-%d INFO \n", buildDate, freeMemory());
  }
}//HeartBeat

//=================================================================================

//<EOF>
