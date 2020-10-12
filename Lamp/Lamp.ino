#include <FastLED.h>
#include "ArduinoQueue.h"

#define NUM_LEDS 256
#define DATA_PIN D2
#define MAX_BRIGHTNESS 10
#define MAX_VOLTS 5
#define MAX_milliAMP 2000

CRGB leds[NUM_LEDS], LastColor_RGB;
CHSV LastColor_HSV;

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
  LastColor_HSV = CHSV(fH, fS, fV);
  return LastColor_HSV;
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
  LastColor_RGB = CRGB(fR, fG, fB);
  return LastColor_RGB;
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

void out(float a, float b, float c, bool f = 1)
{
  Serial.print(a);  Serial.print(" ");
  Serial.print(b);  Serial.print(" ");
  Serial.print(c);  Serial.print(" ");
  if(f) Serial.println();
}

CRGB change_color(CHSV color, float h_change, float s_change = 0.99)
{
  float h = color.h, v = color.v, s = color.s;
  out(h, s, v);
  h +=  h_change;
  if(h < 0) h += 256;
  h = fmod(h, 256);
  LastColor_HSV = CHSV(h, s, v);
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


void start_plus_show(int x, int y, CRGB start_color)
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
        checked[numb] = 1; leds[numb] = change_color(col, 10); make_point(x + 1, y); q2.enqueue(ADD); }
/// 2
    numb = get_number(x - 1, y);
    if(numb != 404)
      if(checked[numb] == 0) {
        checked[numb] = 1; leds[numb] = change_color(col, 10); make_point(x - 1, y); q2.enqueue(ADD); }
/// 3        
    numb = get_number(x, y - 1);
    if(numb != 404)
      if(checked[numb] == 0) {
        checked[numb] = 1; leds[numb] = change_color(col, 10); make_point(x, y - 1); q2.enqueue(ADD); }
/// 4
    numb = get_number(x, y + 1);
    if(numb != 404)
      if(checked[numb] == 0) {
        checked[numb] = 1; leds[numb] = change_color(col, 10); make_point(x, y + 1); q2.enqueue(ADD); }
    
    if(q1.isEmpty())
    {
      while(q2.itemCount() > 0) 
        q1.enqueue(q2.dequeue());
      FastLED.show();
      delay(25);
    }
  }
}

void start_full_matrix_show(CHSV color)
{
  int v = color.v, h = color.h;
  float s = color.s;
  Serial.print(s);
  for(int i(h); i < h + 360; i++)
  {
    FastLED.showColor(HSV_to_RGB(i % 360, s, v));
    delay(250);
  }
}

void build_ball(int& x, int& y)
{
  for(int i(x - 2); i < x + 4; i++)
  {
    if(i < 1 || i > 14) continue;
    for(int j(y - 2); j < y + 4; j++)
      if(j >= 0 && j < 16)
        leds[get_number(i, j)] = CRGB::Black;
  }
  leds[get_number(x, y)] = CRGB::Lime;
  leds[get_number(x, y + 1)] = CRGB::Lime;
  leds[get_number(x + 1, y)] = CRGB::Lime;
  leds[get_number(x + 1, y + 1)] = CRGB::Lime;
}

void add_ball_coord(int& val, int& add)
{
  val += add;
  if(val > 13)  val = 13;
  if(val < 1) val = 1;
  if(val == 13 || val == 1) add *= -1;     
}

void start_ball(int x, int y)
{
  if(x > 14) x = 13;
  if(y > 14) y = 13;
  for(int i(0); i < NUM_LEDS; i++)
    leds[i] = CRGB::Black;
  for(int i(0); i < 16; i++)
  {
    leds[get_number(0, i)] = CRGB::Maroon;
    leds[get_number(15, i)] = CRGB::Maroon;
  }
  build_ball(x, y);
  int add_x = 2, add_y = 1;
  while(1)
  {
    x += add_x;
    if(x > 13)  x = 13;
    if(x < 1) x = 1;
    if(x == 13 || x == 1) add_x *= -1;     
    y += add_y;
    if(y > 14)  y = 14;
    if(y < 0) y = 0;
    if(y == 14 || y == 0) add_y *= -1;     
    build_ball(x, y);
    FastLED.show();
    delay(500); 
  }
}

void BchB(CRGB color1, CRGB color2)
{
  for(int i(0); i < 5; i++)
    for(int j(0); j < 16; j++)
      leds[get_number(i, j)] = color1;
  FastLED.show();
  delay(1500);
  for(int i(5); i < 11; i++)
    for(int j(0); j < 16; j++)
      leds[get_number(i, j)] = color2;
  FastLED.show();
  delay(1500);
  for(int i(11); i < 16; i++)
    for(int j(0); j < 16; j++)
      leds[get_number(i, j)] = color1;
  FastLED.show();
  delay(1500);
}

void Build_BchB()
{
  BchB(CRGB(15, 15, 15), CRGB(15, 0, 0));
  delay(1000);
  BchB(CRGB(255, 255, 255), CRGB(255, 0, 0));
  delay(2000);
  BchB(CRGB::Black, CRGB::Black);
}

void setup() { 
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTS, MAX_milliAMP);
//...................................................
    FastLED.setBrightness(MAX_BRIGHTNESS);
    FastLED.clear();
    Serial.begin(9600);
    srand(time(NULL));
    LastColor_RGB = CRGB::Blue;
    LastColor_HSV = RGB_to_HSV(LastColor_RGB);
}

void loop() { 
  start_plus_show(rand() % 16, rand() % 16, LastColor_RGB);
//  Build_BchB();
//  start_ball(10, 3);
//  start_full_matrix_show(LastColor_HSV);
}
