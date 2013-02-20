/**
 * An Mirf example which copies back the data it recives.
 * While wating the arduino goes to sleep and will be woken up
 * by the interupt pin of the mirf.
 * 
 * Warning: Due to the sleep mode the Serial output donsn't work.
 *
 * Pins:
 * Hardware SPI:
 * MISO -> 12
 * MOSI -> 11
 * SCK -> 13
 *
 * Configurable:
 * CE -> 8
 * CSN -> 7
 */

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

union packed {
  struct test {
    unsigned char cmd;
    float value;
  } floatval;
  struct test3 {
    unsigned char cmd;
    int32_t value;
  } int32val;
  struct test2 {
    unsigned char cmd;
  } cmd;
  unsigned char bytes[8];
} serialpacked;

unsigned long SendTimer;
unsigned long t1, t2;
unsigned char sensid;

void setup(){
  Serial.begin(9600);

  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"clie1");
  Mirf.payload = sizeof(serialpacked);
  Mirf.config();
  
  Serial.println("Listening..."); 

  SendTimer = millis();
  sensid = 1;
}

void loop(){
  byte data[Mirf.payload];
  if((millis() - SendTimer) > 1000) {
    if(!Mirf.isSending()){
      serialpacked.cmd.cmd = sensid;
      sensid++;
      if (sensid > 3) sensid = 1;
      Mirf.setTADDR((byte *)"clie2");
      Mirf.send(serialpacked.bytes);
      t1 = micros();
      //Serial.print("request ");
      //Serial.print(sensid, DEC);
      //Serial.println(" sent");
      SendTimer = millis();
    }
  } 
  if(!Mirf.isSending() && Mirf.dataReady()){
    Mirf.getData(serialpacked.bytes);
    t2 = micros();
    //Serial.print("package! cmd=");
    //Serial.print(serialpacked.cmd.cmd, HEX);
    switch (serialpacked.cmd.cmd) {
      case 1:
        Serial.print("ds  t =  ");
        Serial.print(serialpacked.floatval.value);
        break;
      case 2:
        Serial.print("bmp p = ");
        Serial.print(serialpacked.int32val.value);
        break;
      case 3:
        Serial.print("bmp t =  ");
        Serial.print(serialpacked.floatval.value);
        break;
    }
    Serial.print(" | dt = ");
    Serial.print(t2 - t1);
    Serial.println(" us");
    
  }
}
