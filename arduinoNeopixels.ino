#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

#define PIN      6
#define N_LEDS 64
#define SELECT 13
#define MOVE 12
#define BACK 11

int convertOneDimension(int x, int y);
int convertPixel(int x, int y);

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);
int prevMoveVal = 0;
int count = 0;
long previousMillis = 0;
int ledState = LOW;
int moves[] = {12, 45, 49, 33};
int pixel = moves[0];
int moveIndex = 0;
int previousMoveButton = HIGH;
int previousSelectButton = HIGH;
int previousBack = HIGH;

const uint32_t P1_COLOUR = strip.Color(0,20,20);
const uint32_t P2_COLOUR = strip.Color(0,20,0);
const uint32_t MOVE_COLOUR = strip.Color(25, 10, 0);
SoftwareSerial gtSerial(8, 7);

void setup() {
  strip.begin();
  strip.clear();
  pinMode (SELECT, INPUT);
  pinMode (MOVE, INPUT);
  pinMode (BACK, INPUT);
  Serial.begin(9600);
  gtSerial.begin(9600);

  for(int i = 0; i < sizeof(moves)/sizeof(int); i ++)
  {
    strip.setPixelColor(moves[i], MOVE_COLOUR);
    strip.show();
  }
}

void loop() {
  int moveVal = analogRead(MOVE);
  int selectVal = digitalRead(SELECT);
  int dir = -1;
  int playColour;
  int currentMoveButton;
  int back;
  int sz = sizeof(moves)/sizeof(int);

  currentMoveButton = digitalRead(MOVE);
  back = digitalRead(BACK);

  if(millis() - previousMillis > 250)
  {
    if(ledState == LOW)
    {
      ledState = HIGH;
      strip.setPixelColor(pixel, MOVE_COLOUR);
      strip.show();
    }
    else
    {
      ledState = LOW;
      strip.setPixelColor(pixel, 0);
      strip.show();
    }
    previousMillis = millis();
  }
}

int convertPixel(int x, int y)
{
  int coordinate = convertOneDimension(x, y);

  if(y%2 == 0)
    return coordinate + 2*(4-x)-1;

  return coordinate;
}

int convertPixel(int coordinate)
{
  int x = convertToCoordinateX(coordinate);
  int y = convertToCoordinateY(coordinate);
    
  if(y%2 == 0)
    return coordinate + 2*(4-x)-1;
}

int convertOneDimension(int x, int y)
{
  return 8 * y + x;
}

int incX(int dir)
{
  int x[] = {0, 1, 0, -1};

  if(dir == -1)
    return 0;
  
  return x[dir];
}

int incY(int dir)
{
  int y[] = {-1, 0, 1, 0};

  if(dir == -1)
    return 0;

  return y[dir];
}

int convertToCoordinateX(int coordinate)
{
    return coordinate % 8;
}

int convertToCoordinateY(int coordinate)
{    
    return coordinate / 8;
}



