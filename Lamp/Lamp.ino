#include <FastLED.h>
#include "ArduinoQueue.h"

#define NUM_LEDS 256
#define DATA_PIN D2

CRGB leds[NUM_LEDS], LastColor;

CHSV RGB_to_HSV(CRGB color) {
  float fR = color.r, fG = color.g, fB = color.b;
  float fCMax = max(max(fR, fG), fB),
        fCMin = min(min(fR, fG), fB),
        fDelta = fCMax - fCMin, 
        fH, fS, fV;
  
  if(fDelta > 0) {
    if(fCMax == fR) {
      fH = 60 * (fmod(((fG - fB) / fDelta), 6));
    } else if(fCMax == fG) {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    } else if(fCMax == fB) {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }
    
    if(fCMax > 0) {
      fS = fDelta / fCMax;
    } else {
      fS = 0;
    }
    
    fV = fCMax;
  } else {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }
  
  if(fH < 0) {
    fH += 360;
  }
  return CHSV(fH, fS, fV);
}

CRGB HSV_to_RGB(float fH, float fS, float fV) {
  float fR, fG, fB,
        fC = fV * fS, // Chroma
        fHPrime = fmod(fH / 60.0, 6);
  float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1)),
        fM = fV - fC;
  
  if(0 <= fHPrime && fHPrime < 1) {
    fR = fC;
    fG = fX;
    fB = 0;
  } else if(1 <= fHPrime && fHPrime < 2) {
    fR = fX;
    fG = fC;
    fB = 0;
  } else if(2 <= fHPrime && fHPrime < 3) {
    fR = 0;
    fG = fC;
    fB = fX;
  } else if(3 <= fHPrime && fHPrime < 4) {
    fR = 0;
    fG = fX;
    fB = fC;
  } else if(4 <= fHPrime && fHPrime < 5) {
    fR = fX;
    fG = 0;
    fB = fC;
  } else if(5 <= fHPrime && fHPrime < 6) {
    fR = fC;
    fG = 0;
    fB = fX;
  } else {
    fR = 0;
    fG = 0;
    fB = 0;
  }
  
  fR += fM;
  fG += fM;
  fB += fM;
  LastColor = CRGB(fR, fG, fB);
  return LastColor;
}

inline int get_matrix_number(int x)
{
  if(x % 32 > 15)
    x = (x / 16 + 1) * 16 - x % 16 - 1;
  return x;
}

inline int get_number(int line, int column)
{
  if(line > 15 || line < 0 || column > 15 || column < 0)  return 404;
  return get_matrix_number(line * 16 + column);
}


CRGB change_color(CHSV color)
{
  float h = color.h, v = color.v, s = color.s;
  h += 10;
  if(h > 359) h -= 359;
  return HSV_to_RGB(h, s, v);
}

struct point {
  int X, Y;
};

point ADD;
inline void make_point(int x, int y)
{
  ADD.X = x;
  ADD.Y = y;
}


void setColor(int x, int y, CRGB start_color)
{
  ArduinoQueue<point> q2(NUM_LEDS + 10),
                      q1(NUM_LEDS + 10);
  int numb, checked[NUM_LEDS + 10];
  CHSV col;
  for(int i(0); i < NUM_LEDS; i++)
    checked[i] = 0;
  make_point(x, y);
  q1.enqueue(ADD);
  
  leds[get_number(x, y)] = start_color;
  checked[get_number(x, y)] = 1;
  
  FastLED.show();
  while(q1.itemCount() > 0)
  {
    x = (q1.getHead()).X;
    y = (q1.getHead()).Y;
    q1.dequeue();
    col = RGB_to_HSV(leds[get_number(x, y)]);
/// 1
    numb = get_number(x + 1, y);
    if(numb != 404)
      if(checked[numb] == 0) {
        checked[numb] = 1; leds[numb] = change_color(col); make_point(x + 1, y); q2.enqueue(ADD); }
/// 2
    numb = get_number(x - 1, y);
    if(numb != 404)
      if(checked[numb] == 0) {
        checked[numb] = 1; leds[numb] = change_color(col); make_point(x - 1, y); q2.enqueue(ADD); }
/// 3        
    numb = get_number(x, y - 1);
    if(numb != 404)
      if(checked[numb] == 0) {
        checked[numb] = 1; leds[numb] = change_color(col); make_point(x, y - 1); q2.enqueue(ADD); }
/// 4
    numb = get_number(x, y + 1);
    if(numb != 404)
      if(checked[numb] == 0) {
        checked[numb] = 1; leds[numb] = change_color(col); make_point(x, y + 1); q2.enqueue(ADD); }
    
    if(q1.isEmpty())
    {
//      q1 = q2;
      while(q2.itemCount() > 0) 
        q1.enqueue(q2.dequeue());
      FastLED.show();
      delay(50);
    }
  }
}

void setup() { 
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
//...................................................
    FastLED.setBrightness(5);
    FastLED.clear();
    Serial.begin(9600);
    srand(time(NULL));
    LastColor = CRGB(rand() % 256, rand() % 256, rand() % 256);
//    FastLED.showColor(CRGB(253, 188, 180));
}

void loop() { 
  setColor(rand() % 16, rand() % 16, LastColor);
  setColor(rand() % 16, rand() % 16, LastColor);
}
