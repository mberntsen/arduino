#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1, 177);
IPAddress ipserver(192,168,1,150);
EthernetServer server(80);
EthernetClient client;

unsigned long Teller1, Teller2, Teller3, Teller4;

void setup()
{
  Serial.begin(9600);

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  attachInterrupt(0, tlr1, LOW);
  attachInterrupt(1, tlr2, LOW);
  attachInterrupt(2, tlr3, LOW);
  attachInterrupt(3, tlr4, LOW);
  
}

void tlr1()
{
  Teller1++;
}

void tlr2()
{
  Teller2++;
}

void tlr3()
{
  Teller3++;
}

void tlr4()
{
  Teller4++;
}

void loop()
{
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        if (readString.length() < 100) {
          readString += c; 
        }
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          if (readString.startsWith("GET /send433/")) {
            i = 12;
              j = readString.indexOf('/', i + 1);
              if ((j > -1) && (j < 20)) {
                k = readString.indexOf(' ', j + 1);
                readString.substring(i + 1, j).toCharArray(ch, 5);
                m = atoi(ch);
                readString.substring(j + 1, k).toCharArray(ch, 5);
                n = atoi(ch);
                while (statemachine != 0);
                if (m == 1)
                  if (n == 1)
                    cod = aan1;
                  else
                    cod = uit1;
                if (m == 2)
                  if (n == 1)
                    cod = aan2;
                  else
                    cod = uit2;
                if (m == 3)
                  if (n == 1)
                    cod = aan3;
                  else
                    cod = uit3;
                statemachine = 1;
      
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: text/html");
                client.println("Connnection: close");
                client.println();
                client.print(m);
                client.print(" = ");
                client.println(n);
              }
          }
          if (readString.startsWith("GET /lightsens/")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connnection: close");
            client.println();
            client.print("{\"lightsens\":");
            client.print(light);  
            client.print(",\"avglight\":");
            client.print(avglight);  
            client.println("}");
          }
          if (readString.startsWith("GET /onewire/list/")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connnection: close");
            client.println();
            while(ds.search(addr)) {
              client.print("ROM =");
              for( i = 0; i < 8; i++) {
                client.write(' ');
                client.print(addr[i], HEX);
              }
              client.println();
            }
          }
          if (readString.startsWith("GET /tempsens/ ")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connnection: close");
            client.println();
            client.print("{\"metering\":");
            client.print(Sensor[0].Temperature);  
            client.print(",\"outdoor\":");
            client.print(Sensor[1].Temperature);  
            client.print(",\"indoor_ds\":");
            client.print(nRF_ds_t);  
            client.print(",\"indoor_bmp\":");
            client.print(nRF_bmp_t);  
            client.print(",\"indoor_dht\":");
            client.print(nRF_dht_t);  
            client.println("}");
          }
          if (readString.startsWith("GET /presssens/ ")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connnection: close");
            client.println();
            client.print("{\"indoor\":");
            client.print(nRF_bmp_p);  
            client.println("}");
          }
          if (readString.startsWith("GET /rhsens/ ")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connnection: close");
            client.println();
            client.print("{\"indoor\":");
            client.print(nRF_dht_h);  
            client.println("}");
          }
          if (readString.startsWith("GET /elec/ ")) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connnection: close");
            client.println();
            client.print("{\"L1\":");
            client.print(Teller1);
            client.print(",\"L2\":");
            client.print(Teller2);  
            client.print(",\"L3\":");
            client.print(Teller3);  
            client.print(",\"Solar\":");
            client.print(Teller4);  
            client.println("}");
          }
        }  
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    //Serial.println("client disonnected");
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
  if (!digitalRead(46)) {
    digitalWrite(48, HIGH);
    if (oldbel == false) {
      if (client.connect(ipserver, 80)) {
        client.println("GET /mb/bel.php?tring HTTP/1.0");
        client.println();
      }
    }
    oldbel = true;
  } else {
    digitalWrite(48, LOW);
    oldbel = false;
  }

  //nRF
  
  
  if(!Mirf.isSending() && Mirf.dataReady()){
    Mirf.getData(serialpacked.bytes);
    //t2 = micros();
    Serial.print("nRF receive cmd=");
    Serial.println(serialpacked.cmd.cmd, HEX);
    switch (serialpacked.cmd.cmd) {
      case 1:
        nRF_ds_t = serialpacked.floatval.value;
        //Serial.print("ds  t =  ");
        //Serial.print(serialpacked.floatval.value);
        break;
      case 2:
        nRF_bmp_p = serialpacked.int32val.value;
        //Serial.print("bmp p = ");
        //Serial.print(serialpacked.int32val.value);
        break;
      case 3:
        nRF_bmp_t = serialpacked.floatval.value;
        //Serial.print("bmp t =  ");
        //Serial.print(serialpacked.floatval.value);
        break;
      case 4:
        nRF_dht_h = serialpacked.intval.value;
        //Serial.print("dht h =  ");
        //Serial.print(serialpacked.intval.value);
        break;
      case 5:
        nRF_dht_t = serialpacked.intval.value;
        //Serial.print("dht t =  ");
        //Serial.print(serialpacked.intval.value);
        break;
    }
    LastRxTimer = millis();
    doneDumping = false;
//    Serial.print(" | dt = ");
//    Serial.print(t2 - t1);
//    Serial.println(" us");
    
  }
  if((millis() - SendTimer) > 2000) {
    if(!Mirf.isSending()){
      sensid++;
      if (sensid > 4) sensid = 0;
      serialpacked.cmd.cmd = sensid + 1;
      dumpTmp[0] = 1;
      Mirf.writeRegister(0x05, dumpTmp, 1);
      //Mirf.setTADDR((byte *)"clie2");
      Mirf.send(serialpacked.bytes);
      //t1 = micros();
      Serial.print("nRF transmit ");
      Serial.println(sensid, DEC);
      //Serial.println(" sent");
      
    } else {
      Serial.println('isSending??');
    }    
    SendTimer = millis();
  } 
  if ((millis() - LastRxTimer) > 2500) {
    Serial.println("timeout, reinit");
    Mirf.init();
    Mirf.setRADDR((byte *)"clie1");
    Mirf.setTADDR((byte *)"clie2");
    Mirf.payload = sizeof(serialpacked);
    Mirf.config();
    LastRxTimer = millis();
  }
  
  
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
  /*if((millis() - MCTimer2) > 1000) {
    Serial.println(Serial1.available(), DEC);
    MCTimer2 = millis();
  }*/
  if (Serial1.available() > 0) {
    while (Serial1.available()) {
      MCLine[MCIndex++] = Serial1.read() & 0x7F;
    }
    if (MCIndex == 83) {
      for (i = 0; i < 83; i++) {
        if (MCLine[i] == 13) MCLine[i] == 0;
        if (MCLine[i] == 32) MCLine[i] == 0;
      }
      MC_dE = atol(&MCLine[2]) * 0.01;
      MC_T1 = atol(&MCLine[26]) * 0.01;
      MC_T2 = atol(&MCLine[34]) * 0.01;
      MC_P = atol(&MCLine[50]) * 0.1;
      MC_F = atol(&MCLine[58]) * 1.0;
      
      //Serial.println(MC_dE);
      /*MC_dE = Serial1.parseFloat() / 1;
      Serial1.parseFloat();
      Serial1.parseFloat();
      MC_T1 = Serial1.parseFloat() / 1;
      MC_T2 = Serial1.parseFloat() / 1;
      Serial1.parseFloat();
      MC_P = Serial1.parseFloat() / 1;
      MC_F = Serial1.parseFloat();
      Serial1.parseFloat();*/
    }
  }
}

ISR(PCINT2_vect) {    // Interrupt service routine. Every single PCINT8..14 (=ADC0..5) change
            // will generate an interrupt: but this will always be the same interrupt routine
      Serial.print("pcint");
  if (digitalRead(15)) {
      Serial.print("pcint15");
    WaterCounter++;
  }
}
