#include <FastLED.h>

#define NUM_LEDS 256
#define DATA_PIN D2

CRGB leds[NUM_LEDS];

int get_matrix_number(int x)
{
  if(x % 32 > 15)
    x = (x / 16 + 1) * 16 - x % 16 - 1;
  return x;
}

void setup() { 
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
//...................................................
    FastLED.setBrightness(10);
    FastLED.clear();
    Serial.begin(9600);
}

void loop() { 
  
}
