#include <Adafruit_NeoPixel.h>

#define PIN 4
#define STRIPSIZE 49 // Limited by max 256 bytes ram. At 3 bytes/LED you get max ~85 pixels

#define PIN_ENCODER_A 0
#define PIN_ENCODER_B 3// 1 also is the internal LED, better do not use as internal input...
#define PIN_BUTTON 2

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPSIZE, PIN, NEO_GRB + NEO_KHZ800);

boolean LEDsActive = false;
boolean LEDsDirty = false;
int mainLEDsSelected = 0;
int mainLEDsColors[] = {0, 65535/2};

void setup()
{
  // TODO: read last values from memory on startup
  
  // set pins as input with internal pull-up resistors enabled
  pinMode(PIN_ENCODER_A, INPUT);
  pinMode(PIN_ENCODER_B, INPUT);
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_ENCODER_A, HIGH);
  digitalWrite(PIN_ENCODER_B, HIGH);
  digitalWrite(PIN_BUTTON, HIGH);
  
  strip.begin();
  strip.setBrightness(250); // set accordingly
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  checkButtonInput();
  checkEncoderInput();

  if (LEDsActive && LEDsDirty){
    updateMainLEDs();
  }

  updateStatusLED();

  if (LEDsDirty){
    strip.show();
    LEDsDirty = false;
  }  
}

void updateStatusLED()
{
  static unsigned long int lastUpdate = 0;
  static int brightness = 0;
  static int direction = 1;

  if ((millis() - lastUpdate) > (1000 / 24)){ 
    lastUpdate = millis();
    
    if (brightness >= 255){
      direction = -1;
    }
    
    if (brightness < 128){
      direction = 1;
    }
  
    brightness = (brightness + direction);
    strip.setPixelColor(0, brightness, brightness, brightness);
    LEDsDirty = true;
  }
}

void updateMainLEDs()
{
  for (int mainUnit=0; mainUnit<2; mainUnit++){
    for (int led=(1+(mainUnit*24)); led < (((mainUnit+1)*24)+1); led++){
      strip.setPixelColor(led, strip.ColorHSV(mainLEDsColors[mainUnit])); // uint32_t rgbcolor = strip.ColorHSV(hue, saturation, value);
    }
  }
  LEDsDirty = true;
}


//   _____ _____ _____ _____ _____ _____ 
//  | __  |  |  |_   _|_   _|     |   | |
//  | __ -|  |  | | |   | | |  |  | | | |
//  |_____|_____| |_|   |_| |_____|_|___|
//  

void checkButtonInput()
{
  int longPushDelay = 1000;
  int buttonState = digitalRead(PIN_BUTTON);
  static unsigned long pushedDownTimestamp = 0;
  unsigned long pushDuration = 0;
  
  bool shortPushed = false;
  bool longPushed = false;
  static bool stupidFlagINeed = false;

  if(pushedDownTimestamp != 0){
    // we are waiting for release or longpress, so we need the duration
    pushDuration = millis() - pushedDownTimestamp;
  }

  // just found out the button is pressed
  if (buttonState == 0 && pushedDownTimestamp == 0){
    pushedDownTimestamp = millis();
  }

  // button is pressed for a while now
  if (buttonState == 0 && pushedDownTimestamp != 0){
    longPushed = pushDuration >= longPushDelay;
  }

  // button has just been released
  if (buttonState == 1 && pushedDownTimestamp != 0){
    pushedDownTimestamp = 0;
    shortPushed = (pushDuration < longPushDelay) && (pushDuration > 4);
    stupidFlagINeed = false;
  }
  

  if (longPushed && !stupidFlagINeed){
    stupidFlagINeed = true;
    // do something on long push
  
  } else if (shortPushed){
    if (!LEDsActive){
      LEDsActive = true;
      LEDsDirty = true;
    } else {
      mainLEDsSelected = (mainLEDsSelected == 0) ? 1 : 0;
    }
  }
}
 
void checkEncoderInput()
{
  int encoderPos = 0;
  int encoderPinA = digitalRead(PIN_ENCODER_A);
  int encoderPinB = digitalRead(PIN_ENCODER_B);

  static int encoderPinALast = HIGH;
  
  if ((encoderPinALast == LOW) && (encoderPinA == HIGH)) {
    if (encoderPinB == HIGH) {
      encoderPos++;
    } else {
      encoderPos--;
    }
  }
  
  encoderPinALast = encoderPinA;

  if (encoderPos != 0) {
    int newVal = mainLEDsColors[mainLEDsSelected] + (encoderPos * 200);
    if (newVal > 65535){ newVal = newVal % 65535; }
    if (newVal < 0){ newVal = 65535 + newVal; }
    mainLEDsColors[mainLEDsSelected] = newVal;
    LEDsDirty = true;
  }
  
}
