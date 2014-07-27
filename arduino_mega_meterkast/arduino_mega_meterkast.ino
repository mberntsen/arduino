#include "TimerOne.h"
#include <SPI.h>
//#include <Ethernet.h>
#include <Wire.h>
#include <OneWire.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

typedef struct {
  uint8_t Addr[8];
  int16_t RawBuffer[10];
  float   Temperature;
} Sensor_t;

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

Sensor_t Sensor[2] ={
  {{0x28, 0x0E, 0x1E, 0xEA, 0x03, 0x00, 0x00, 0x65},{0,0,0,0,0,0,0,0,0,0},},  // Outside
  {{0x28, 0xFC, 0x9C, 0x8E, 0x04, 0x00, 0x00, 0xD5},{0,0,0,0,0,0,0,0,0,0},}  // Outside
};

int rxPin = 9;
int txPin = 6;
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
//IPAddress ip(192,168,1, 177);
//IPAddress ipserver(192,168,1,150);
byte buff[2];
byte wirestate = 0;
unsigned long wiret;
//EthernetServer server(80);
//EthernetClient client;
OneWire  ds(5);
byte RawIndex = 0;
unsigned int DSState;
unsigned long DSTimer;
byte addr[8];
byte data[12];
int t;
unsigned char aan1[8] = {0x01, 0x8F, 0x36, 0xCE, 0x99, 0x75, 0xAE, 0x47};
unsigned char uit1[8] = {0x01, 0x8F, 0x36, 0xCE, 0x99, 0x75, 0xAD, 0x47};
unsigned char aan2[8] = {0x01, 0x8F, 0x36, 0xCE, 0x99, 0x75, 0xAE, 0x49};
unsigned char uit2[8] = {0x01, 0x8F, 0x36, 0xCE, 0x99, 0x75, 0xAD, 0x49};
unsigned char aan3[8] = {0x01, 0x8F, 0x36, 0xCE, 0x99, 0x75, 0xAE, 0x4B};
unsigned char uit3[8] = {0x01, 0x8F, 0x36, 0xCE, 0x99, 0x75, 0xAD, 0x4B};


/*
// 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 
char aan1[57] = {1,1,0,0,0,1,1,1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,1,0,0,1,0,0,0,1,1,1};
char uit1[57] = {1,1,0,0,0,1,1,1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,0,0,1,1,1};
char aan2[57] = {1,1,0,0,0,1,1,1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,1,0,0,1,0,0,1,0,1,1};
char uit2[57] = {1,1,0,0,0,1,1,1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,0,1,0,1,1};
char aan3[57] = {1,1,0,0,0,1,1,1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,1,0,0,1,0,0,1,1,1,1};
char uit3[57] = {1,1,0,0,0,1,1,1,1,0,0,1,1,0,1,1,0,1,1,0,0,1,1,1,0,1,0,0,1,1,0,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,0,1,0,1,0,0,1,1,1,1};
*/
unsigned char code433[8];
unsigned char *cod;
volatile char statemachine = 0;
unsigned char heBitcount;
unsigned char heSendcount;
unsigned char heBytecount;
unsigned char heBitmask;
String readString;
int i, j, k, m, n;
boolean valfound;
char ch[17];
unsigned int light;
unsigned int oldlight[12];
unsigned long avglight;
byte wirei;
boolean isdark, oldisdark;
boolean oldbel;
unsigned long SendTimer;
unsigned char sensid;
float nRF_ds_t, nRF_bmp_t, nRF_ds2_t;
unsigned long nRF_bmp_p;
float nRF_dht_h, nRF_dht_t;
boolean nRF_ds_t_valid;
boolean nRF_ds2_t_valid;
unsigned long nRF_ds_t_last;
unsigned long nRF_ds2_t_last;
unsigned long last_time_not_too_cold;
float t_setpoint;
float MC_dE, MC_T1, MC_T2, MC_P, MC_F;
//unsigned long MC_dE, MC_T1, MC_T2, MC_P, MC_F;
unsigned long MCTimer, MCTimer2;
char MCLine[92];
unsigned char MCIndex;
boolean doneDumping = true;
unsigned long LastRxTimer;
unsigned char dumpTmp[5];
boolean forcenRFDump = true;
volatile unsigned long WaterCounter, WaterdtTimer, WaterTimerOld;
volatile unsigned long PowerCounter1, PowerCounter2, PowerCounter3, PowerCounter4;
volatile unsigned long PowerdtTimer1, PowerdtTimer2, PowerdtTimer3, PowerdtTimer4;
volatile unsigned long PowerTimer1Old, PowerTimer2Old, PowerTimer3Old, PowerTimer4Old;
unsigned char OldPINB, OldPINK;

void setup()
{
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  pinMode(46, OUTPUT);
  pinMode(47, OUTPUT);
  pinMode(48, OUTPUT);
  pinMode(49, OUTPUT);

  pinMode(10, INPUT);
  pinMode(11, INPUT);
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  pinMode(A8, INPUT);
  
  pinMode(15, INPUT);     //set the pin to input
  digitalWrite(15, LOW); //use the internal pullup resistor
  cli();		// switch interrupts off while messing with their settings  
  PCICR  = 0b00000101;          // Enable PCINT1 interrupt
  PCMSK0 = 0b11110000;
  PCMSK2 = 0b00000001;
  sei();		// turn interrupts back on
  OldPINB = PINB;
  OldPINK = PINK;
  Wire.begin();
  //Serial.begin(9600);

  Serial3.begin(115200);

  Mirf.spi = &MirfHardwareSpi;
//  Mirf.cePin = 44;
//  Mirf.csnPin = 42;
  Mirf.init();
  Mirf.setRADDR((byte *)"clie1");
  Mirf.setTADDR((byte *)"clie2");
  Mirf.payload = sizeof(serialpacked);
  Mirf.config();
  
  //Ethernet.begin(mac, ip);
  //server.begin();
  //Serial3.print("server is at ");
  //Serial3.println(Ethernet.localIP());

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timercallback);
  
  SendTimer = millis();
  MCTimer = millis();
  MCTimer2 = millis();
  LastRxTimer = millis();
  //doneDumping = true;
  nRF_ds_t_valid = false;
  t_setpoint = 12;
  
}

/*void timercallback()
{
  switch (statemachine) {
  case 1:
    heSendcount = 0;
  case 2:
    digitalWrite(txPin, HIGH);
    Timer1.setPeriod(250);
    statemachine = 3;
    heBitcount = 0;
    break;
  case 3:
    digitalWrite(txPin, LOW);
    Timer1.setPeriod(10000);
    statemachine = 4;
    break;
  case 4:
    digitalWrite(txPin, HIGH);
    Timer1.setPeriod(250);
    if (heBitcount < 57)
      statemachine = 5;
    else
      statemachine = 6;
    break;
  case 5:  
    digitalWrite(txPin, LOW);
    if (cod[heBitcount] == 0)
      Timer1.setPeriod(200);
    else
      Timer1.setPeriod(1000);
    heBitcount++;
    statemachine = 4;
    break;
  case 6:  
    digitalWrite(txPin, LOW);
    heSendcount++;
    Timer1.setPeriod(1000);
    if (heSendcount < 3)
      statemachine = 2;
    else
      statemachine = 0;
    break;
  }
}*/

//nieuw
void timercallback()
{
  switch (statemachine) {
  case 1:
    heSendcount = 0;
  case 2:
    digitalWrite(txPin, HIGH);
    Timer1.setPeriod(250);
    statemachine = 3;
    heBitmask = 0x01;
    heBytecount = 0;
    break;
  case 3:
    digitalWrite(txPin, LOW);
    Timer1.setPeriod(10000);
    statemachine = 4;
    break;
  case 4:
    digitalWrite(txPin, HIGH);
    Timer1.setPeriod(250);
    if (heBytecount < 8)
      statemachine = 5;
    else
      statemachine = 6;
    break;
  case 5:  
    digitalWrite(txPin, LOW);
    if ((cod[heBytecount] & heBitmask) == 0)
      Timer1.setPeriod(200);
    else
      Timer1.setPeriod(1000);
    heBitmask >>= 1;
    if (heBitmask == 0) {
      heBytecount++;
      heBitmask = 0x80;
    }
    statemachine = 4;
    break;
  case 6:  
    digitalWrite(txPin, LOW);
    heSendcount++;
    Timer1.setPeriod(1000);
    if (heSendcount < 3)
      statemachine = 2;
    else
      statemachine = 0;
    break;
  }
}

char sch;
unsigned long tempLong;

void loop()
{
  //Serial3.println(Serial3.available());
  
  while (Serial3.available()) {
    sch = Serial3.read();
    //Serial3.print(sch);
    switch (sch) {
      case 2:
        readString = "";
        break;
      case 3:
        //Serial3.println(readString);
        if (readString.startsWith("GET /send433/")) {
          if (statemachine == 0) {
            if (readString.length() >= 30) {
              i = 12;
              readString.substring(13).toCharArray(ch, 17);
              memset(code433, 0, 8);
              for (i = 0; i < 16; i++) {
                switch(ch[i]) {
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                    code433[i >> 1] += ch[i] - '0';
                    break;
                  case 'a':
                  case 'b':
                  case 'c':
                  case 'd':
                  case 'e':
                  case 'f':
                    code433[i >> 1] += ch[i] - 'a' + 10;
                    break;
                  case 'A':
                  case 'B':
                  case 'C':
                  case 'D':
                  case 'E':
                  case 'F':
                    code433[i >> 1] += ch[i] - 'A' + 10;
                    break;
                }
                if (i % 2 == 0) code433[i >> 1] <<= 4;
              } 
              /*for (i = 0; i < 16; i++)
                Serial3.print(ch[i], HEX);
              Serial3.print(' ');
              for (i = 0; i < 8; i++) {
                //Serial3.print(aan1[i], HEX);
                Serial3.print(code433[i], HEX);
              }*/             
              cod = code433;
              statemachine = 1;
              Serial3.print("OK\x03");
            } 
              Serial3.print("invalid code\x03");
          } else
            Serial3.print("busy\x03");
        }
        if (readString.startsWith("GET /lightsens/")) {
          Serial3.print("{\"lightsens\":");
          Serial3.print(light);  
          Serial3.print(",\"avglight\":");
          Serial3.print(avglight);  
          Serial3.print("}");
          Serial3.print("\x03");
        }
        if (readString.startsWith("GET /onewire/list/")) {
          while(ds.search(addr)) {
            Serial3.print("ROM =");
            for( i = 0; i < 8; i++) {
              Serial3.write(' ');
              Serial3.print(addr[i], HEX);
            }
          Serial3.print("\x03");
          }
        }
        if (readString.startsWith("GET /tempsens/")) {
          Serial3.print("{\"metering\":");
          Serial3.print(Sensor[0].Temperature);  
          Serial3.print(",\"outdoor\":");
          Serial3.print(Sensor[1].Temperature);  
          Serial3.print(",\"indoor_ds\":");
          Serial3.print(nRF_ds_t);  
          Serial3.print(",\"indoor_ds2\":");
          Serial3.print(nRF_ds2_t);  
          Serial3.print(",\"indoor_bmp\":");
          Serial3.print(nRF_bmp_t);  
          Serial3.print(",\"indoor_dht\":");
          Serial3.print(nRF_dht_t);  
          Serial3.print("}");
          Serial3.print("\x03");
        }
        if (readString.startsWith("GET /presssens/")) {
          Serial3.print("{\"indoor\":");
          Serial3.print(nRF_bmp_p);  
          Serial3.print("}");
          Serial3.print("\x03");
        }
        if (readString.startsWith("GET /rhsens/")) {
          Serial3.print("{\"indoor\":");
          Serial3.print(nRF_dht_h);  
          Serial3.print("}");
          Serial3.print("\x03");
        }
        if (readString.startsWith("GET /cv/")) {
          Serial3.print("{\"state\":");
          Serial3.print(digitalRead(47));
          Serial3.print(",\"ts\":");
          Serial3.print(t_setpoint);  
          Serial3.print(",\"warmte\":");
          Serial3.print(MC_dE);  
          Serial3.print(",\"T1\":");
          Serial3.print(MC_T1);  
          Serial3.print(",\"T2\":");
          Serial3.print(MC_T2);  
          Serial3.print(",\"Power\":");
          Serial3.print(MC_P);  
          Serial3.print(",\"Flow\":");
          Serial3.print(MC_F);  
          Serial3.print("}");
          Serial3.print("\x03");
        }
        if (readString.startsWith("GET /water/")) {
          Serial3.print("{\"w\":");
          cli();
          tempLong = WaterCounter;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"wt\":");  
          cli();
          tempLong = WaterdtTimer;
          sei();
          Serial3.print(tempLong);  
          Serial3.print("}");
          Serial3.print("\x03");
        }
        if (readString.startsWith("SET /water/")) {
          i = 11;
          j = readString.indexOf('/', i);
          if (j > -1) {
            readString.substring(i, j).toCharArray(ch, 10);
            tempLong = atol(ch);
            cli();
            WaterCounter = tempLong;
            sei();
            Serial3.print("OK\x03");
          }
        }
        if (readString.startsWith("GET /power/")) {
          Serial3.print("{\"f1\":");
          cli();
          tempLong = PowerCounter1;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"f2\":");  
          cli();
          tempLong = PowerCounter2;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"f3\":");  
          cli();
          tempLong = PowerCounter3;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"pv\":");  
          cli();
          tempLong = PowerCounter4;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"f1t\":");  
          cli();
          tempLong = PowerdtTimer1;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"f2t\":");  
          cli();
          tempLong = PowerdtTimer2;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"f3t\":");  
          cli();
          tempLong = PowerdtTimer3;
          sei();
          Serial3.print(tempLong);  
          Serial3.print(",\"pvt\":");  
          cli();
          tempLong = PowerdtTimer4;
          sei();
          Serial3.print(tempLong);  
          Serial3.print("}");
          Serial3.print("\x03");
        }
        if (readString.startsWith("SET /power/1/")) {
          i = 13;
          j = readString.indexOf('/', i);
          if (j > -1) {
            readString.substring(i, j).toCharArray(ch, 10);
            tempLong = atol(ch);
            cli();
            PowerCounter1 = tempLong;
            sei();
            Serial3.print("OK\x03");
          }
        }
        if (readString.startsWith("SET /power/2/")) {
          i = 13;
          j = readString.indexOf('/', i);
          if (j > -1) {
            readString.substring(i, j).toCharArray(ch, 10);
            tempLong = atol(ch);
            cli();
            PowerCounter2 = tempLong;
            sei();
            Serial3.print("OK\x03");
          }
        }
        if (readString.startsWith("SET /power/3/")) {
          i = 13;
          j = readString.indexOf('/', i);
          if (j > -1) {
            readString.substring(i, j).toCharArray(ch, 10);
            tempLong = atol(ch);
            cli();
            PowerCounter3 = tempLong;
            sei();
            Serial3.print("OK\x03");
          }
        }
        if (readString.startsWith("SET /power/4/")) {
          i = 13;
          j = readString.indexOf('/', i);
          if (j > -1) {
            readString.substring(i, j).toCharArray(ch, 10);
            tempLong = atol(ch);
            cli();
            PowerCounter4 = tempLong;
            sei();
            Serial3.print("OK\x03");
          }
        }
        if (readString.startsWith("SET /tsetpoint/")) {
          i = 15;
          j = readString.indexOf('/', i);
          if (j > -1) {
            readString.substring(i, j).toCharArray(ch, 10);
            t_setpoint = atol(ch) / 10.0;
            Serial3.print("{\"ts\":");
            Serial3.print(t_setpoint);  
            Serial3.print("}");
            Serial3.print("\x03");
          }
        }
        if (readString.startsWith("GET /tsetpoint/")) {
          Serial3.print("{\"ts\":");
          Serial3.print(t_setpoint);  
          Serial3.print("}");
          Serial3.print("\x03");
        }

        if (readString.startsWith("GET /nrfdump/")) {
          Mirf.readRegister(0x00, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x01, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x02, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x03, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x04, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x05, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x06, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x07, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x08, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x09, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x0A, dumpTmp, 5);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(dumpTmp[1], HEX);
      Serial3.print(dumpTmp[2], HEX);
      Serial3.print(dumpTmp[3], HEX);
      Serial3.print(dumpTmp[4], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x0B, dumpTmp, 5);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(dumpTmp[1], HEX);
      Serial3.print(dumpTmp[2], HEX);
      Serial3.print(dumpTmp[3], HEX);
      Serial3.print(dumpTmp[4], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x0C, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x0D, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x0E, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x0F, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x10, dumpTmp, 5);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(dumpTmp[1], HEX);
      Serial3.print(dumpTmp[2], HEX);
      Serial3.print(dumpTmp[3], HEX);
      Serial3.print(dumpTmp[4], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x11, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x12, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x13, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x14, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x15, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x16, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x17, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x1C, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
      Serial3.print(' ');
      Mirf.readRegister(0x1D, dumpTmp, 1);
      Serial3.print(dumpTmp[0], HEX);
          Serial3.print("\x03");
        }
        break;
      default:
        readString += sch;
        break;
    }
  }
  
  //I2C
  switch (wirestate) {
  case 0:
    Wire.beginTransmission(0x23);
    Wire.write(0x11);
    Wire.endTransmission();
    wiret = millis() + 200; 
    wirestate = 1;
    break;
  case 1:
    if (millis() > wiret) wirestate = 2;
    break;
  case 2:
    Wire.beginTransmission(0x23);
    Wire.requestFrom(0x23, 2);
    light = 256 * Wire.read();
    light += Wire.read();
    Wire.endTransmission();
    wirei++;
    if (wirei > 11) wirei = 0;
    oldlight[wirei] = light;
    avglight = 0;
    for (i = 0; i < 12; i++) 
      avglight += oldlight[i];
    if (avglight < 36)
      isdark = true;
    if (avglight > 60)
      isdark = false;
    if ((isdark == true) && (oldisdark == false)) {
      while (statemachine != 0);
      cod = aan1;
      statemachine = 1;
    }
    if ((isdark == false) && (oldisdark == true)) {
      while (statemachine != 0);
      cod = uit1;
      statemachine = 1;
    }
    oldisdark = isdark;
    wiret += 5000;
    wirestate = 1;
    break;
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
      for(i = 0; i < 2; i++){
        ds.reset();
        ds.select(Sensor[i].Addr);    
        ds.write(0xBE);             // Read Scratchpad
        for(j = 0; j < 2; j++) {    // we need 9 bytes
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
    }
    break;
  }
  
  //bel
  if (!digitalRead(45)) {
    digitalWrite(46, HIGH);
    if (oldbel == false) {
      //if (client.connect(ipserver, 80)) {
        //client.println("GET /mb/bel.php?tring HTTP/1.0");
        //client.println();
      //}
    }
    oldbel = true;
  } else {
    digitalWrite(45, LOW);
    oldbel = false;
  }

  //nRF
  if(!Mirf.isSending() && Mirf.dataReady()){
    Mirf.getData(serialpacked.bytes);
    //Serial3.print("nRF receive cmd=");
    //Serial3.println(serialpacked.cmd.cmd, HEX);
    switch (serialpacked.cmd.cmd) {
      case 1:
        nRF_ds_t = serialpacked.floatval.value;
        nRF_ds_t_last = millis();
        if (nRF_ds_t_valid == false) {
          last_time_not_too_cold = millis();
        }
        nRF_ds_t_valid = true;
        //Serial3.print("ds  t =  ");
        //Serial3.print(serialpacked.floatval.value);
        break;
      case 2:
        nRF_bmp_p = serialpacked.int32val.value;
        //Serial3.print("bmp p = ");
        //Serial3.print(serialpacked.int32val.value);
        break;
      case 3:
        nRF_bmp_t = serialpacked.floatval.value;
        //Serial3.print("bmp t =  ");
        //Serial3.print(serialpacked.floatval.value);
        break;
      case 4:
        nRF_dht_h = serialpacked.floatval.value;
        //Serial3.print("dht h =  ");
        //Serial3.print(serialpacked.intval.value);
        break;
      case 5:
        nRF_dht_t = serialpacked.floatval.value;
        //Serial3.print("dht t =  ");
        //Serial3.print(serialpacked.intval.value);
        break;
      case 6:
        nRF_ds2_t = serialpacked.floatval.value;
        nRF_ds2_t_last = millis();
        nRF_ds2_t_valid = true;
        //Serial3.print("ds2  t =  ");
        //Serial3.print(serialpacked.floatval.value);
        break;
    }
    LastRxTimer = millis();
    doneDumping = false;
//    Serial3.print(" | dt = ");
//    Serial3.print(t2 - t1);
//    Serial3.println(" us");
    
  }
  if ((millis() - nRF_ds_t_last) > 30000) {
    nRF_ds_t_valid = false;
    digitalWrite(47, LOW);
  }
  if((millis() - SendTimer) > 1000) {
    if(!Mirf.isSending()){
      sensid++;
      if (sensid > 5) sensid = 0;
      serialpacked.cmd.cmd = sensid + 1;
      dumpTmp[0] = 1;
      Mirf.writeRegister(0x05, dumpTmp, 1);
      //Mirf.setTADDR((byte *)"clie2");
      Mirf.send(serialpacked.bytes);
      //Serial3.print("nRF transmit ");
      //Serial3.println(sensid, DEC);
      
    } else {
      //Serial3.println('isSending??');
    }    
    SendTimer = millis();
  } 
  /*if ((millis() - LastRxTimer) > 2500) {
    //Serial3.println("timeout, reinit");
    Mirf.init();
    Mirf.setRADDR((byte *)"clie1");
    Mirf.setTADDR((byte *)"clie2");
    Mirf.payload = sizeof(serialpacked);
    Mirf.config();
    LastRxTimer = millis();
  }*/
  
  
  //stadsverwarmingding
  if((millis() - MCTimer) > 20000) {
    
    Serial1.end();
    Serial1.begin(300);
    Serial1.println("/#1");
    Serial1.end();
    Serial1.begin(1200);
    MCTimer = millis();
    MCIndex = 0;
  }
  if (Serial1.available() > 0) {
    while (Serial1.available()) {
      MCLine[MCIndex++] = Serial1.read() & 0x7F;
    }
    if (MCIndex == 83) {
      for (i = 0; i < 83; i++) {
        if (MCLine[i] == 13) MCLine[i] == 0;
        if (MCLine[i] == 32) MCLine[i] == 0;
      }
      // ~104 us without floats
      // ~155 us with floats
      MC_dE = atol(&MCLine[2]) * 0.01;
      MC_T1 = atol(&MCLine[26]) * 0.01;
      MC_T2 = atol(&MCLine[34]) * 0.01;
      MC_P = atol(&MCLine[50]) * 0.1;
      MC_F = atol(&MCLine[58]) * 1.0;
    }
  }
  
  if (nRF_ds_t_valid) {
    if (nRF_ds_t < t_setpoint) {
      //if (nRF_ds2_t_valid) {
        if ((millis() - last_time_not_too_cold) > 60000) {
          digitalWrite(47, HIGH);
        }
      //}
    } else {
      last_time_not_too_cold = millis();
      digitalWrite(47, LOW);
    }
  }
}

//10 bruin pv
//11 groen f3
//12 geel  f1
//13 grijs f2
ISR(PCINT0_vect) { 
  unsigned char dpb = (PINB ^ OldPINB) & PINB;
  OldPINB = PINB;
  if ((dpb & 0x10) == 0x10) {
    PowerCounter4++;
    PowerdtTimer4 = millis() - PowerTimer4Old;
    PowerTimer4Old = millis();
  }
  if ((dpb & 0x20) == 0x20) {
    PowerCounter3++;
    PowerdtTimer3 = millis() - PowerTimer3Old;
    PowerTimer3Old = millis();
  }
  if ((dpb & 0x40) == 0x40) {
    PowerCounter1++;
    PowerdtTimer1 = millis() - PowerTimer1Old;
    PowerTimer1Old = millis();
  }
  if ((dpb & 0x80) == 0x80) {
    PowerCounter2++;
    PowerdtTimer2 = millis() - PowerTimer2Old;
    PowerTimer2Old = millis();
  }
}

ISR(PCINT2_vect) {
  unsigned char dpk = (PINK ^ OldPINK);
  OldPINK = PINK;
  //Serial3.print(dpk, HEX);
  if ((dpk & 0x01) == 0x01) {
    WaterCounter++;
    WaterdtTimer = millis() - WaterTimerOld;
    WaterTimerOld = millis();
    //Serial3.print(dpk, HEX);
  }
  
}
