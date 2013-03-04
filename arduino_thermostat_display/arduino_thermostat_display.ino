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
unsigned long DSTimer;
byte addr[8];
byte data[12];
int i, j;
unsigned long t0;
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
//char * floatToString(char * outstr, float value, int places, int minwidth, bool rightjustify);

// The setup() method runs once, when the sketch starts
void setup()   {                
  Serial.begin(9600);

  pinMode(BACKLIGHT_LED, OUTPUT);
  digitalWrite(BACKLIGHT_LED, HIGH);

  glcd.st7565_init();
  glcd.st7565_command(CMD_SET_BIAS_9);
  glcd.st7565_command(CMD_DISPLAY_ON);
  glcd.st7565_command(CMD_SET_ALLPTS_NORMAL);
  glcd.st7565_set_brightness(0x0C);

  //uint8_t i;
  //for (i=0; i < 128; i++) {
  //  glcd.drawchar((i % 21) * 6, i/21, 167);
  //}
  //Serial.println(i, DEC);
  //while(1);
  glcd.display(); // show splashscreen
  //while(1);
  //glcd.clear();
  
  while(ds.search(addr)) {
    Serial.print("ROM =");
    for( i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
    }
    Serial.println();
  }

  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  }
  
  t0 = millis();
}

void loop()                     
{
  if (millis() - t0 > 1000) {
    strcpy(test, "actual:   ");
    floatToString(&test[10], Sensor[0].Temperature, 1, 5, true);
    strcat (test,"\xA7""C");
    //test[16] = -89;
    glcd.drawstring(6, 1, test);
    strcpy(test, "setpoint: ");
    floatToString(&test[10], setpoint/10, 1, 5, true);
    strcat (test,"\xA7""C");
    //test[16] = -89;
    glcd.drawstring(6, 2, test);
    glcd.display();
    t0 = millis();
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
        for(j = 0; j < 9; j++) {    // we need 9 bytes
          data[j] = ds.read();
        }
        Sensor[i].RawBuffer[RawIndex] = (data[1] << 8) | data[0];
        Sensor[i].Temperature = 0;
        for(j = 0; j < 10; j++) {    // Gemiddelde berekenen
          Sensor[i].Temperature += (float)Sensor[i].RawBuffer[j];
        }
        Sensor[i].Temperature /= 160;  // Origineel: Sensor[i].Temperature = ((float)raw / 16.0);
      }
      if(++RawIndex > 9) RawIndex = 0;
      DSState = 0;
      //Serial.print("measured=");
      //Serial.println(Sensor[0].Temperature);
    }
    break;
  }
}

// floatToString.h
//
// Tim Hirzel
// tim@growdown.com
// March 2008
// float to string
// 
// If you don't save this as a .h, you will want to remove the default arguments 
//     uncomment this first line, and swap it for the next.  I don't think keyword arguments compile in .pde files

char * floatToString(char * outstr, float value, int places, int minwidth, bool rightjustify) {
//char * floatToString(char * outstr, float value, int places, int minwidth=0, bool rightjustify=false) {
    // this is used to write a float value to string, outstr.  oustr is also the return value.
    int digit;
    float tens = 0.1;
    int tenscount = 0;
    int i;
    float tempfloat = value;
    int c = 0;
    int charcount = 1;
    int extra = 0;
    // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
    // if this rounding step isn't here, the value  54.321 prints as 54.3209

    // calculate rounding term d:   0.5/pow(10,places)  
    float d = 0.5;
    if (value < 0)
        d *= -1.0;
    // divide by ten for each decimal place
    for (i = 0; i < places; i++)
        d/= 10.0;    
    // this small addition, combined with truncation will round our values properly 
    tempfloat +=  d;

    // first get value tens to be the large power of ten less than value    
    if (value < 0)
        tempfloat *= -1.0;
    while ((tens * 10.0) <= tempfloat) {
        tens *= 10.0;
        tenscount += 1;
    }

    if (tenscount > 0)
        charcount += tenscount;
    else
        charcount += 1;

    if (value < 0)
        charcount += 1;
    charcount += 1 + places;

    minwidth += 1; // both count the null final character
    if (minwidth > charcount){        
        extra = minwidth - charcount;
        charcount = minwidth;
    }

    if (extra > 0 and rightjustify) {
        for (int i = 0; i< extra; i++) {
            outstr[c++] = ' ';
        }
    }

    // write out the negative if needed
    if (value < 0)
        outstr[c++] = '-';

    if (tenscount == 0) 
        outstr[c++] = '0';

    for (i=0; i< tenscount; i++) {
        digit = (int) (tempfloat/tens);
        itoa(digit, &outstr[c++], 10);
        tempfloat = tempfloat - ((float)digit * tens);
        tens /= 10.0;
    }

    // if no places after decimal, stop now and return

    // otherwise, write the point and continue on
    if (places > 0)
    outstr[c++] = '.';


    // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
    for (i = 0; i < places; i++) {
        tempfloat *= 10.0; 
        digit = (int) tempfloat;
        itoa(digit, &outstr[c++], 10);
        // once written, subtract off that digit
        tempfloat = tempfloat - (float) digit; 
    }
    if (extra > 0 and not rightjustify) {
        for (int i = 0; i< extra; i++) {
            outstr[c++] = ' ';
        }
    }


    outstr[c++] = '\0';
    return outstr;
}
