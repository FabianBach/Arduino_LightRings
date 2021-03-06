#include <Adafruit_NeoPixel.h>

enum input: byte {
  COLOR,
  SETTINGS,
  EFFECTS
};

input selectedInput = COLOR;

enum settings: byte {
  MODE,
  BRIGHTNESS
};

settings selectedSetting = BRIGHTNESS;
int mainBrightness = 100;

enum modes: byte {
  INDIVIDUAL,
  COMPLEMENTARY
};

modes selectedMode = INDIVIDUAL;

enum effects: byte {
  GRADIENT,
  PULSE
};

effects selectedEffect = GRADIENT;
int effectIntensityGradient = 500;
int effectIntensityPulse = 0;

#define PIN_INTERNAL_LED 1
#define PIN_LEDS 4
#define NUM_LEDS 49 // Limited by max 256 bytes ram. At 3 bytes/LED you get max ~85 pixels

#define PIN_ENCODER_A 0
#define PIN_ENCODER_B 3// 1 also is the internal LED, better do not use as internal input...
#define PIN_BUTTON 2

boolean inputsDirty = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN_LEDS, NEO_GRB + NEO_KHZ800);

boolean LEDsActive = false;
boolean LEDsDirty = false;
int mainLEDsSelected = 0;
int mainLEDsColors[] = {0, 65535/3};

void setup()
{
  // TODO: read last values from memory on startup

  pinMode(PIN_INTERNAL_LED, OUTPUT);
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

  digitalWrite(PIN_INTERNAL_LED, inputsDirty ? HIGH : LOW);

  if (LEDsActive && inputsDirty){
    updateMainLEDs();
  }

  updateStatusLED();

  if (LEDsDirty){
    strip.show();
    LEDsDirty = false;
  }  

  inputsDirty = false;
}


//   __    _____ ____  _____ 
//  |  |  |   __|    \|   __|
//  |  |__|   __|  |  |__   |
//  |_____|_____|____/|_____|
//  

void updateStatusLED()
{
  static unsigned long int lastUpdate = 0;
  static int brightness = 0;
  int pixelColor = 0;
  static int direction = 1;

  if ((millis() - lastUpdate) > (1000 / 24)){ 
    lastUpdate = millis();

    brightness = (brightness + (direction*10));
    
    if (brightness >= 255){
      brightness = 255;
      direction = -1;
    }
    
    if (brightness < 50){
      brightness = 50;
      direction = 1;
    }

    switch(selectedInput){
      case COLOR:
        pixelColor = int(65535/6);
        break;
        
      case SETTINGS:
        pixelColor = int(65535);
        break;
        
      case EFFECTS:
        pixelColor = int(65535/2);
        break;
    }
    
    strip.setPixelColor(0, strip.ColorHSV(pixelColor, 255, brightness));
    LEDsDirty = true;
  }
}

void updateMainLEDs()
{
  for (int mainUnit=0; mainUnit<2; mainUnit++){

    int baseColor = (selectedMode == COMPLEMENTARY) ? ((mainLEDsColors[0] + ((65535/2)*mainUnit))%65535) : mainLEDsColors[mainUnit];
    
    for (int led = 0; led < 24; led++){
      int pixelGradientMultiplicator = (led < 11) ? led : (23 - led);
      int pixelColor = (baseColor + (effectIntensityGradient * pixelGradientMultiplicator))%65535;

      strip.setPixelColor(1+(mainUnit*24)+led, strip.gamma32(strip.ColorHSV(pixelColor, 255, mainBrightness))); 
      // TODO: maybe use: strip.gamma32(strip.ColorHSV(hue, sat, val));
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

    switch(selectedInput){
      case COLOR:
        selectedInput = SETTINGS;
        break;

      case SETTINGS:
        selectedInput = EFFECTS;
        break;
      
      case EFFECTS:
        selectedInput = COLOR;
        break;
    }

    inputsDirty = true;
  
  } else if (shortPushed){

    switch(selectedInput){
      case COLOR:
        if (!LEDsActive){
          LEDsActive = true;
        } else {
          mainLEDsSelected = (mainLEDsSelected == 0) ? 1 : 0;
        }
        break;

      case SETTINGS:
        selectedSetting = (selectedSetting == MODE) ? BRIGHTNESS : MODE;
        break;
      
      case EFFECTS:
        //selectedEffect = (selectedEffect == PULSE) ? GRADIENT : PULSE;
        break;
    }

    inputsDirty = true;
  }
}


//   _____ _____ _____ _____ ____  _____ _____ 
//  |   __|   | |     |     |    \|   __| __  |
//  |   __| | | |   --|  |  |  |  |   __|    -|
//  |_____|_|___|_____|_____|____/|_____|__|__|
//
 
void checkEncoderInput()
{
  int encoderPos = 0;
  int encoderPinA = digitalRead(PIN_ENCODER_A);
  int encoderPinB = digitalRead(PIN_ENCODER_B);

  static int encoderPinALast = HIGH;
  
  if ((encoderPinALast == LOW) && (encoderPinA == HIGH)) {
    if (encoderPinB == HIGH) {
      encoderPos--;
    } else {
      encoderPos++;
    }
  }
  
  encoderPinALast = encoderPinA;

  if (encoderPos != 0) {

    switch(selectedInput){
      case COLOR: {
        int selectedLED = (selectedMode == INDIVIDUAL) ? mainLEDsSelected : 0;
        int newVal = mainLEDsColors[selectedLED] + (encoderPos * 200);
        if (newVal > 65535){ newVal = newVal % 65535; }
        if (newVal < 0){ newVal = 65535 + newVal; }
        mainLEDsColors[selectedLED] = newVal;
        }
        break;

      case SETTINGS:
        switch(selectedSetting){
          case MODE:
            selectedMode = (encoderPos > 0) ? COMPLEMENTARY : INDIVIDUAL;
            break;
            
          case BRIGHTNESS:
            mainBrightness = mainBrightness + (encoderPos * 10);
            if (mainBrightness < 0) {mainBrightness = 0;}
            if (mainBrightness > 255) {mainBrightness = 255;}
            break;
        }
        break;
      
      case EFFECTS:
        switch(selectedEffect){
          case GRADIENT:
            effectIntensityGradient = effectIntensityGradient + (encoderPos * 50);
            if (effectIntensityGradient < 0) {effectIntensityGradient = 0;}
            break;
    
//          case PULSE:
//            effectIntensityPulse = effectIntensityPulse + (encoderPos * 10);
//            if (effectIntensityPulse < 0) {effectIntensityPulse = 0;}
//            break;
        }
        break;
    }
    
    inputsDirty = true;
  }
}
