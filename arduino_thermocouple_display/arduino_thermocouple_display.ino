// (c) Tom Soulanille - public domain!
// Modified from:
// (c) adafruit industries - public domain!

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <OneWire.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "ST7565.h"
#include <string.h>

typedef struct {
  uint8_t Addr[8];
  int16_t RawBuffer[10];
  float   Temperature;
} Sensor_t;

Sensor_t Sensor[2] ={
  {{0x28, 0xEC, 0x21, 0xEA, 0x03, 0x00, 0x00, 0xC9},{0,0,0,0,0,0,0,0,0,0},}  // Outside
};

OneWire  ds(A0);
int ledPin =  13;    // LED connected to digital pin 13
byte RawIndex = 0;
unsigned int DSState;
unsigned long DSTimer, HTimer;
byte addr[8];
byte data[12];
int i, j;
unsigned long t0, t1, dt;
char test[25];
unsigned int setpoint=200;

// the LCD backlight is connected up to a pin so you can turn it on & off
#define BACKLIGHT_LED 10

// pin 9 - Serial data out (SID)
// pin 8 - Serial clock out (SCLK)
// pin 7 - Data/Command select (RS or A0)
// pin 6 - LCD reset (RST)
// pin 5 - LCD chip select (CS)
ST7565 glcd(9, 8, 7, 6, 5);
Adafruit_BMP085 bmp;

// the buffer for the image display
extern uint8_t st7565_buffer[1024];

float a1, a2, a3, a4, a5, a6, a7, a8, a9;
volatile float v1, v2, v3, v4, v5, v6, v7, v8, v9, Tm, Ta, Tsp=50;
char in;
int m[50];
long msum;
boolean turnon = false;
  
// The setup() method runs once, when the sketch starts
void setup()   {                
  Serial.begin(9600);

  pinMode(BACKLIGHT_LED, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(BACKLIGHT_LED, HIGH);

  glcd.st7565_init();
  glcd.st7565_command(CMD_SET_BIAS_9);
  glcd.st7565_command(CMD_DISPLAY_ON);
  glcd.st7565_command(CMD_SET_ALLPTS_NORMAL);
  glcd.st7565_set_brightness(0x0C);

  glcd.display(); // show splashscreen
  
  a1 = 2.508355e-2;
  a2 = 7.860106e-8;
  a3 = -2.503131e-10;
  a4 = 8.315270e-14;
  a5 = -1.228034e-17;
  a6 = 9.804036e-22;
  a7 = -4.413030e-26;
  a8 = 1.057734e-30;
  a9 = -1.052755e-35;
  
  in = 0;
  
  t0 = millis();
  HTimer = millis();
}

void loop()                     
{
  int an1, an2, an3;
  an1 = analogRead(A1);
  an2 = analogRead(A2);
  m[in++] = an2 - an1;
  if (in == 100) {
    msum = 0;
    for(i = 0; i < 50; i++) msum += m[i];
    msum /= 100;
    v1 = msum * (3222.65625 / 150);
    v2 = v1 * v1;
    v3 = v2 * v1;
    v4 = v3 * v1;
    v5 = v4 * v1;
    v6 = v5 * v1;
    v7 = v6 * v1;
    v8 = v7 * v1;
    v9 = v8 * v1;
    Tm = Ta + (a1 * v1) + (a2 * v2) + (a3 * v3) + (a4 * v4) + (a5 * v5) + (a6 * v6) + (a7 * v7) + (a8 * v8) + (a9 * v9);
    t1 = millis();
    dt = t1 - t0;
    //t0 = t1;
    in = 0;
  
    strcpy(test, "Ta : ");
    dtostrf(Ta,3,0,&test[5]);
    strcat (test," \xA7""C");
    glcd.drawstring(6, 1, test);
    strcpy(test, "Tsp: ");
    dtostrf(Tsp,3,0,&test[5]);
    strcat (test," \xA7""C");
    glcd.drawstring(6, 2, test);
    strcpy(test, "Tm : ");
    dtostrf(Tm,3,0,&test[5]);
    strcat (test," \xA7""C");
    glcd.drawstring(6, 3, test);
    sprintf(test, "dt : %3lu ms", dt);
    glcd.drawstring(6, 4, test);
    if (turnon)
      sprintf(test, "out:  on");
    else
      sprintf(test, "out: off");
    glcd.drawstring(6, 5, test);
    
    glcd.display();
    t0 = millis();
  }
  
  if ((millis() - HTimer) > 10000) {
    //Serial.println(1.25);
    turnon = (Tsp > Tm);
    digitalWrite(13, turnon);
    HTimer = millis();
  }
  
  //onewire
  switch (DSState){
  case 0:
    ds.reset();
    ds.write(0xCC);         // skip ROM
    ds.write(0x44);         // start conversion
    DSTimer = millis();
    DSState++;
    break;
  case 1:
    if((millis() - DSTimer) >= 1000){
      for(i = 0; i < 1; i++){
        ds.reset();
        ds.select(Sensor[i].Addr);    
        ds.write(0xBE);             // Read Scratchpad
        for(j = 0; j < 2; j++) {    // we need 9 bytes
          data[j] = ds.read();
        }
        Ta = ((data[1] << 8) | data[0]) / 16.0;
        Sensor[i].Temperature = 0;
      }
      DSState = 0;
    }
    break;
  }
}

