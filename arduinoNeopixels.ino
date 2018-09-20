// Simple NeoPixel test.  Lights just a few pixels at a time so a
// 1m strip can safely be powered from Arduino 5V pin.  Arduino
// may nonetheless hiccup when LEDs are first connected and not
// accept code.  So upload code first, unplug USB, connect pixels
// to GND FIRST, then +5V and digital pin 6, then re-plug USB.
// A working strip will show a few pixels moving down the line,
// cycling between red, green and blue.  If you get no response,
// might be connected to wrong end of strip (the end wires, if
// any, are no indication -- look instead for the data direction
// arrows printed on the strip).

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
  //chase(strip.Color(255, 0, 0)); // Red
  //chase(strip.Color(0, 255, 0)); // Green
  //chase(strip.Color(0, 0, 255)); // Blue

  int moveVal = analogRead(MOVE);
  int selectVal = digitalRead(SELECT);
  //int curMillis;
  int dir = -1;
  int playColour;
  int currentMoveButton;
  int back;
  int sz = sizeof(moves)/sizeof(int);
  /*
  if(playValue == 1)
    playColour = P1_COLOUR;
  else if(playValue == -1)
    playColour = P2_COLOUR;
    */



  currentMoveButton = digitalRead(MOVE);
  back = digitalRead(BACK);

/*
  if(back == LOW && previousBack == HIGH)
  {
    moveIndex--;

    if(moveIndex < 0)
      moveIndex = sz - 1;

    strip.setPixelColor(pixel, MOVE_COLOUR);
    strip.show();
    pixel = moves[moveIndex];
  }
  
  else if(currentMoveButton == LOW && previousMoveButton == HIGH)
  {
    moveIndex++;

    if(moveIndex >= sz)
      moveIndex = 0;

    strip.setPixelColor(pixel, MOVE_COLOUR);
    strip.show();
    pixel = moves[moveIndex];
  }
  */

/*
  if(moveVal >= 950 && moveVal <= 1024)
  {
    if(prevMoveVal >= 950 && prevMoveVal <= 1024)
      count++;
    else
      count = 0;
    if(count >= 30)
    {
      dir = 0;
      count = 0;
      strip.setPixelColor(pixel, 0);
      strip.show();
    }
  }
  else if(moveVal >= 585 && moveVal <= 640)
  {
    if(prevMoveVal >= 585 && prevMoveVal <= 640)
      count++;
    else
      count = 0;
    if(count >= 30)
    {
      dir = 1;
      count = 0;
      strip.setPixelColor(pixel, 0);
      //Serial.print("current: ");
      //Serial.println(pixel);
      strip.show();
    }
  }
  else if(moveVal >= 500 && moveVal <= 560)
  {
    if(prevMoveVal >= 500 && prevMoveVal <= 560)
      count++;
    else
      count = 0;
    if(count >= 30)
    {
      dir = 2;
      count = 0;
      strip.setPixelColor(pixel, 0);
      strip.show();
    }
  }
  else if(moveVal >= 20 && moveVal <= 55)
  {
    //Serial.println("asdsadasdasdsad");
    //delay(100);
    if(prevMoveVal >= 20 && prevMoveVal <= 55)
      count++;
    else
      count = 0;
    if(count >= 30)
    {
      dir = 3;
      count = 0;
      strip.setPixelColor(pixel, 0);
      strip.show();
    }
  }
  else
  {
    count = 0;
  }
*/
  //Serial.println(dir);

//  xCoordinate += incX(dir);
//  yCoordinate += incY(dir);

/*
  if(dir > -1)
  {
    //Serial.println(incX(1));
    delay(1000);
  }
  */

//  pixel = convertPixel(xCoordinate, yCoordinate);
  
  //prevMoveVal = moveVal;
  //Serial.println(pixel);

  if(millis() - previousMillis > 250)
  {
    //Serial.println(pixel);
    //Serial.println("asdsad");
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


  /*
  if(selectVal == LOW && previousSelectButton == HIGH)
  {
    //Select
    strip.setPixelColor(pixel, playColour);
    strip.show();
    
  }

  previousMoveButton = currentMoveButton;
  previousBack = back;
  previousSelectButton = selectVal;
  */
}

int convertPixel(int x, int y)
{
  int coordinate = convertOneDimension(x, y);
  //int conversion[] = {7, 5, 3, 1, -1, -3, -5, -7}

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



