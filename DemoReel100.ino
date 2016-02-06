#include "FastLED.h"
FASTLED_USING_NAMESPACE;

// FastLED "100-lines-of-code" demo reel, showing just a few
// of the kinds of animation patterns you can quickly and easily
// compose using FastLED.
//
// This example also shows one easy way to define multiple
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    3
//#define CLK_PIN   4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    300
CRGB leds[NUM_LEDS];

int button = D6;

int brightness = 96;
#define FRAMES_PER_SECOND  120

// IDE tries to add prototypes to the top, before #includes, which screws up everything.
// Manually add a prototype so it doesnt.
// Can that possibly be right?
void addGlitter( fract8 chanceOfGlitter);

int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);

void setup() {
  delay(3000); // 3 second delay for recovery
  pinMode(button, INPUT_PULLDOWN);
  Spark.function("digitalread", tinkerDigitalRead);
  Spark.function("digitalwrite", tinkerDigitalWrite);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(brightness);

  Spark.publish("DEBUG", "Hello World");
  //attachInterrupt(D2, brightnessUp, FALLING);
}

int tinkerDigitalRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT_PULLDOWN);
		return digitalRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT_PULLDOWN);
		return digitalRead(pinNumber+10);
	}
	return -2;
}

int tinkerVal = 0;
int tinkerDigitalWrite(String command)
{
	if(command.substring(3,7) == "HIGH") tinkerVal = 1;
	else if(command.substring(3,6) == "LOW") tinkerVal = 0;
	else return -2;

  return 1;
}

void brightnessUp() {
  nextPattern();
  //brightness += 10;
  //Serial.print(brightness);
  //FastLED.setBrightness(brightness);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { white, red, green, blue, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, bpm2 };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

int buttonState = 0;
int tinkerValState = 0;
void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  //FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  //EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

EVERY_N_SECONDS( 1 ) {
  Spark.publish("DEBUG", Time.timeStr() + " " + String(millis())); 
}

  int b = digitalRead(button);
  if ( b != buttonState ) {
    if ( buttonState == 0) {
      nextPattern();
    }
    buttonState = b;
    Spark.publish("DEBUG", String(b));
    //Serial.print(b);
    //Serial.print("banana");
  }

  if ( tinkerVal != tinkerValState ) {
    nextPattern();
    tinkerValState = tinkerVal;

    //Serial.print(b);
    //Serial.print("banana");
  }

  //if ( digitalRead(button) == HIGH ) {
//    nextPattern();
//  }
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  FastLED.show();
}

void white()
{
  fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
}

void red()
{
  fill_solid( leds, NUM_LEDS, CRGB(255,0,0));
}

void green()
{
  fill_solid( leds, NUM_LEDS, CRGB(0, 255, 0));
}

void blue()
{
  fill_solid( leds, NUM_LEDS, CRGB(0,0,255));
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}


void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 120;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void bpm2()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  fadeToBlackBy( leds, NUM_LEDS, 80);
  uint16_t BeatsPerMinute = 800 << 8;
  uint8_t middle = (NUM_LEDS + 1)/2;
  uint8_t beat = beatsin88( BeatsPerMinute, 10, middle);
  for( int i = middle-beat; i < beat+middle; i++) { //9948
    leds[i] = leds[NUM_LEDS - i - 1] = CHSV( gHue, 255, 192);
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
