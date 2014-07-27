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
#include <OneWire.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <dht22.h>

union packed {
  struct test {
    unsigned char cmd;
    float value;
  } floatval;
  struct test3 {
    unsigned char cmd;
    int32_t value;
  } int32val;
  struct test4 {
    unsigned char cmd;
    int value;
  } intval;
  struct test2 {
    unsigned char cmd;
  } cmd;
  unsigned char bytes[8];
} serialpacked;

typedef struct {
  uint8_t Addr[8];
  int16_t RawBuffer[10];
  float   Temperature;
} Sensor_t;

Sensor_t Sensor[2] ={
  {{0x28, 0x71, 0x39, 0xEA, 0x03, 0x00, 0x00, 0x70},{0,0,0,0,0,0,0,0,0,0}, 0.0f},
  {{0x28, 0x57, 0x02, 0xEA, 0x03, 0x00, 0x00, 0x80},{0,0,0,0,0,0,0,0,0,0}, 0.0f}  // Outside
};
/*Sensor_t Sensor[2] ={
  {{0x28, 0x71, 0x39, 0xEA, 0x03, 0x00, 0x00, 0x70},{0,0,0,0,0,0,0,0,0,0},}  // Outside
};*/

OneWire  ds(5);
Adafruit_BMP085 bmp;
dht22 DHT22;

#define DHT22PIN 4

byte RawIndex = 0;
unsigned int DSState, dhtState;
unsigned long DSTimer;
unsigned long dht22Timer;
byte addr[8];
byte data[12];
int i, j;

void setup(){
  Serial.begin(9600);

  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"clie2");
  Mirf.payload = sizeof(serialpacked);
  Mirf.config();
  
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
  dht22Timer = millis();
}

void loop(){
  byte data[Mirf.payload];
  
  if(!Mirf.isSending() && Mirf.dataReady()){
  //if(Mirf.dataReady()){
    Mirf.getData(serialpacked.bytes);
    //Serial.print("cmd=");
    //Serial.println(serialpacked.cmd.cmd, HEX);
    switch (serialpacked.cmd.cmd) {
      case 0:
        oneWireSync = 1;
        bmpSync = 1;
        dhtSync = 1;
        break;
/*      case 1:
        serialpacked.floatval.cmd = 1;
        serialpacked.floatval.value = Sensor[0].Temperature;
        Mirf.setTADDR((byte *)"clie1");
        Mirf.send(serialpacked.bytes);
        //Serial.println("temperature sent");
        break;
      case 2:
        //Serial.print(bmp.readTemperature());
        serialpacked.int32val.cmd = 2;
        serialpacked.int32val.value = bmp.readPressure();
        Mirf.setTADDR((byte *)"clie1");
        Mirf.send(serialpacked.bytes);
        break;
      case 3:
        //Serial.print(bmp.readTemperature());
        serialpacked.floatval.cmd = 3;
        serialpacked.floatval.value = bmp.readTemperature();
        Mirf.setTADDR((byte *)"clie1");
        Mirf.send(serialpacked.bytes);
        break;
      case 4:
        //Serial.print(bmp.readTemperature());
        serialpacked.floatval.cmd = 4;
        serialpacked.floatval.value = DHT22.humidity;
        Mirf.setTADDR((byte *)"clie1");
        Mirf.send(serialpacked.bytes);
        break;
      case 5:
        //Serial.print(bmp.readTemperature());
        serialpacked.floatval.cmd = 5;
        serialpacked.floatval.value = DHT22.temperature;
        Mirf.setTADDR((byte *)"clie1");
        Mirf.send(serialpacked.bytes);
        break;
      case 6:
        serialpacked.floatval.cmd = 6;
        serialpacked.floatval.value = Sensor[1].Temperature;
        Mirf.setTADDR((byte *)"clie1");
        Mirf.send(serialpacked.bytes);
        //Serial.println("temperature sent");
        break;
*/      
    }
  }
  //onewire
  switch (DSState){
  case 0:
    if (oneWireSync) {
      DSState = 1;
      oneWireSync = 0;
    }
    break;
  case 1:
    ds.reset();
    ds.write(0xCC);         // skip ROM
    ds.write(0x44);         // start conversion
    DSTimer = millis();
    DSState = 2;
    break;
  case 2:
    if((millis() - DSTimer) >= 1000){
      for(i = 0; i < 2; i++){
        ds.reset();
        ds.select(Sensor[i].Addr);    
        ds.write(0xBE);             // Read Scratchpad
        for(j = 0; j < 9; j++) {    // we need 9 bytes
          data[j] = ds.read();
        }
        //Sensor[i].RawBuffer[RawIndex] = (data[1] << 8) | data[0];
        //Sensor[i].Temperature = 0;
        //for(j = 0; j < 10; j++) {    // Gemiddelde berekenen
        //  Sensor[i].Temperature += (float)Sensor[i].RawBuffer[j];
        //}
        //Sensor[i].Temperature /= 160;  // Origineel: Sensor[i].Temperature = ((float)raw / 16.0);
        Sensor[i].Temperature = ((data[1] << 8) | data[0]) / 16;
//        Serial.print("measured=");
//        Serial.println(Sensor[i].Temperature);
      }
      //if(++RawIndex > 9) RawIndex = 0;
      DSState = 3;
    }
    break;
  case 3:
    if(!Mirf.isSending()){
      serialpacked.floatval.cmd = 1;
      serialpacked.floatval.value = Sensor[0].Temperature;
      Mirf.setTADDR((byte *)"clie1");
      Mirf.send(serialpacked.bytes);
      DSState = 4;
    }
  case 4:
    if(!Mirf.isSending()){
      serialpacked.floatval.cmd = 6;
      serialpacked.floatval.value = Sensor[1].Temperature;
      Mirf.setTADDR((byte *)"clie1");
      Mirf.send(serialpacked.bytes);
      DSState = 0;
    }
  }

  //dht11
  //if ((millis() - dht22Timer) >= 1000) {
  //  dht22Timer = millis();
  //  int chk = DHT22.read(DHT22PIN);
  switch (dhtState) {
  case 0:
    if (dhtSync) {
      dhtSync = 0;
      dhtState = 1;
    }
    break;
  case 1:
    int chk = DHT22.read(DHT22PIN);
    dhtState = 2;
    break;
  case 2:  
    if(!Mirf.isSending()){
      serialpacked.floatval.cmd = 4;
      serialpacked.floatval.value = DHT22.humidity;
      Mirf.setTADDR((byte *)"clie1");
      Mirf.send(serialpacked.bytes);
      dhtState = 3;
    }
    break;
  case 3:
    //Serial.print(bmp.readTemperature());
    if(!Mirf.isSending()){
      serialpacked.floatval.cmd = 5;
      serialpacked.floatval.value = DHT22.temperature;
      Mirf.setTADDR((byte *)"clie1");
      Mirf.send(serialpacked.bytes);
      dhtState = 0;
    }
  }
}
