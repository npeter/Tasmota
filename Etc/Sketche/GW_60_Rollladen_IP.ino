//GW_60_Rollladen_IP.ino

// ... einmalig erforderliche User-Einstellungen

String      NAME        = "WEMOS_";             // Name des Wemos D1 mini
int         WIP         = XXX;                  // Unique des Namens -->letzte Dreiergruppe der Wunsch-IP (XXX.ZZZ.YYY.145)
const char* mqtt_server = "192.178.0.ZZZ";      // ZZZ --> IP auf der FHEM und Mosquitto läuft
const char* ssid        = "Deine SSID";         // Eigene SSID
const char* password    = "Dein WLAN Passwort"; // WLAN-Passwort
const char* start       = "Pi";                 // Name frei wählbar

//Gateway
int IP1    = 192;                           // 1. Stelle IP - Adresse
int IP2    = 178;                           // 2. Stelle IP - Adresse
int IP3    = 0;                             // 3. Stelle IP - Adresse
int GWY    = 1;                             // 4. Stelle IP - Adresse Gateway

// DNS
int DN1    = 255;                           // 1. Stelle DNS
int DN2    = 255;                           // 2. Stelle DNS
int DN3    = 255;                           // 3. Stelle DNS
int DN4    = 0;                             // 4. Stelle DNS

// ENDE der erforderlichen individuellen User-Einstellungen !!!!!

#define SENSOR1       2   // D4   wird auf LOW gezogen wenn Rollo schliesst
#define SENSOR2       0   // D3   wird auf LOW gezogen wenn Rollo oeffnet
#define SENSOR3      14   // D5   Aktive-Low; REED KONTAKT
#define SENSOR4      16   // D0   Aktive-Low; TASTE STORAGE
#define AKTOR1        5   // D1   geht auf LOW wenn Rollo schliessen soll - liegt parallel zu Taste DOWN
#define AKTOR2        4   // D2   geht auf LOW wenn Rollo oeffnen soll    - liegt parallel zu Taste UP

// Folgende Zeilen m�ssen normal nicht ge�ndert werden !!!!!!!

String CLIENT, ClientID, ZIEL1, ZIEL2, ZIEL3, ZIEL4, ZIEL5, QUELLE1, QUELLE2;
#include <Ticker.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>   //--> Achtung unbedingt Version 2.6 installieren

WiFiClient ESPClient;             
PubSubClient client(ESPClient);  
Ticker tickerSENSOR1;
Ticker tickerSENSOR2;
Ticker tickerSENSOR3;
Ticker tickerSENSOR4;

int Time_SENSOR      = 60;     // ALife-Intervall 
int ON_Zeit          = 1000;   // ON-Zeit der Aktoren in ms
int doSENSOR1        = 0;
int doSENSOR2        = 0;
int doSENSOR3        = 0;
int doSENSOR4        = 0;
int prevSENSOR1      = 0;
int prevSENSOR2      = 0;
int prevSENSOR3      = 0;
int prevSENSOR4      = 0;
int set_Down         = 0;
int set_Up           = 0;
int set_TUp          = 0;
int set_TDown        = 0;
int preset_TUp       = 0;
int preset_TDown     = 0;
int Pos              = 75;
int pre_Pos          = 75;
int maxPos           = 150;
int pre_maxPos       = 150;
int step             = 1;
char msg[5];
char strTD[6];
const char* ID = NAME.c_str();      // Clientname Konstante

void heartBeat(int doPin)
{
 switch (doPin)
  {
   case SENSOR1:
   doSENSOR1 = 1;
   break;
   case SENSOR2:
   doSENSOR2 = 1;
   break;
   case SENSOR3:
   doSENSOR3 = 1;
   break;
   case SENSOR4:
   doSENSOR4 = 1;
   break;
  }
}
void setup()
{
 //Serial.begin(115200);
 //serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
 //Serial.setDebugOutput(true);
 CLIENT   = ID;
 ClientID = CLIENT + WIP;                // ClientID
 ZIEL1    = ClientID + "/Schliessen";    // sendet Rollo schlie�t
 ZIEL2    = ClientID + "/Oeffnen";       // sendet Rollo �ffnet
 ZIEL3    = ClientID + "/Position";      // sendet aktuelle Rolloposition
 ZIEL4    = ClientID + "/Position_%";    // sendet Rolloposition in %
 ZIEL5    = ClientID + "/maxPosition";   // sendet Rollo maximale Position
 QUELLE1  = ClientID + "/Down";          // empf�ngt Taste DOWN
 QUELLE2  = ClientID + "/Up";            // empf�ngt Taste UP

 pinMode(SENSOR1,      INPUT_PULLUP);
 pinMode(SENSOR2,      INPUT_PULLUP);
 pinMode(SENSOR3,      INPUT_PULLUP);
 pinMode(SENSOR4,      INPUT_PULLUP);
 pinMode(AKTOR1,       OUTPUT);
 pinMode(AKTOR2,       OUTPUT);
 digitalWrite(AKTOR1,  HIGH);
 digitalWrite(AKTOR2,  HIGH);
 setup_wifi();
 client.setServer(mqtt_server, 1883);
 client.setCallback(callback);
     
 tickerSENSOR1.attach(Time_SENSOR, heartBeat, SENSOR1);
 tickerSENSOR2.attach(Time_SENSOR, heartBeat, SENSOR2);
 tickerSENSOR3.attach(Time_SENSOR, heartBeat, SENSOR3);
 tickerSENSOR4.attach(Time_SENSOR, heartBeat, SENSOR4);
 
 EEPROM.begin(2);
 Pos = EEPROM.read(0);
 maxPos = EEPROM.read(1);
 pre_maxPos = maxPos;
 pre_Pos = Pos;
 EEPROM.commit();
 EEPROM.end();
}
void setup_wifi()
{
 WiFi.mode(WIFI_STA);
 //Serial.println("");
 //Serial.println();
 //Serial.print("...Verbinde zu ");
 //Serial.print(ssid);
 //Serial.println("");
 //Serial.println();
 WiFi.begin(ssid, password);
 WiFi.config(IPAddress(IP1,IP2,IP3,WIP), IPAddress(IP1,IP2,IP3,GWY), IPAddress(DN1,DN2,DN3,DN4), IPAddress(IP1,IP2,IP3,GWY)); //Feste IP-Vergabe
 while (WiFi.status() != WL_CONNECTED)
 {  
  delay(500);
  //Serial.print(".");
 }
  //Serial.print("...WiFi-Verbindung über ");
  //Serial.print("IP-Adresse: ");
  //Serial.print(WiFi.localIP());
  //Serial.println(" erfolgreich !");
}
void callback(char* kanal, byte* nachrichtInBytes, unsigned int length)
{ 

//---- PIN liegt parallel zum Down-Taster --------------------------------------------------------------

String nachricht1 = "";
{
 for (int i = 0; i < length; i++) 
 {  
 nachricht1 += (char)nachrichtInBytes[i];
 }
  String  stringKanal         = kanal;
  String  stringAnmeldeKanal1 = QUELLE1;
  if (stringKanal == stringAnmeldeKanal1)
  {
   if (nachricht1 == "on")
   {  
    set_TUp = 0;
    set_TDown = 0;
    if (set_TDown == preset_TDown) set_TDown = 1;
    if (set_TDown == preset_TUp) set_TDown = 0;
    preset_TDown = set_TDown;
    digitalWrite(AKTOR1, LOW);       // --> entspricht "Down-Taster"
    //Serial.print("...sende von ");
    //Serial.print(QUELLE1);
    //Serial.println("      :  Down ");
    //Serial.print("");
    delay(ON_Zeit);   
    nachricht1 == "Off";
    digitalWrite(AKTOR1, HIGH);
    //Serial.print("...sende von ");
    //Serial.print(QUELLE1);
    //Serial.println("      :  Off  ");
    //Serial.print("");
   }
  }
}

//---- PIN liegt parallel zum Up-Taster --------------------------------------------------------------

String nachricht2 = "";
{
 for (int i = 0; i < length; i++) 
  {  
   nachricht2 += (char)nachrichtInBytes[i];
  }
  String  stringKanal         = kanal;
  String  stringAnmeldeKanal1 = QUELLE2;
  if (stringKanal == stringAnmeldeKanal1)
  {
   if (nachricht2 == "on")
   {  
    set_TDown = 0;
    set_TUp = 0;
    if (set_TUp == preset_TUp) set_TUp = 1;
    if (set_TUp == preset_TDown) set_TUp = 0;
    preset_TUp = set_TUp;
    digitalWrite(AKTOR2, LOW);       // --> entspricht "Up-Taster"
    //Serial.print("...sende von ");
    //Serial.print(QUELLE2);
    //Serial.println("      :  Up ");
    //Serial.print("");
    delay(ON_Zeit);   
    nachricht2 == "Off";
    digitalWrite(AKTOR2, HIGH);
    //Serial.print("...sende von ");
    //Serial.print(QUELLE2);
    //Serial.println("      :  Off  ");
    //Serial.print("");
   }
  }
 }
}
void reconnect() 
{
 while (!client.connected()) 
 {
  //Serial.println("");
  //Serial.print("...warte auf MQTT-Anmeldung.....");
  //Serial.print("");
  //Serial.println();
  if (client.connect(CLIENT.c_str()))
  {
   //Serial.print("");
   //Serial.print("...empfange Anmeldung erfolgreich an :  ");
   client.publish(start, (CLIENT.c_str()));
   //Serial.print(start);
   //Serial.print("  ");
   //Serial.println((CLIENT.c_str()));
   //Serial.print(" ");
   //Serial.println(" ");
   //Serial.print("");
   client.subscribe(QUELLE1.c_str());
   client.subscribe(QUELLE2.c_str());
  } 
   else 
  {
   //Serial.print("Fehler, rc=");
   //Serial.print(client.state());
   //Serial.println(" ...versuche es in 5 Sekunden nochmal !");
   delay(5000);
  }
 }
}
void loop()
{
 if (!client.connected())     
  {
   reconnect();              
  }
  client.loop();               
  
//------ PIN ist LOW während der Rollo schließt ----------------   
  
  int sensor1 = digitalRead(SENSOR1);
  if (sensor1 == 0)
   {
    set_Up = 0;
    set_Down = 1;               
   }
  if (doSENSOR1 || sensor1 != prevSENSOR1)
  {
   const char *state  = (sensor1) ? "Stop" : "Close";
   client.publish((ZIEL1.c_str()),(state));
   //Serial.print("...sende an ");
   //Serial.print(ZIEL1);
   //Serial.print(" :  ");
   //Serial.println(state);
   dtostrf(Pos ,3,0, msg);
   client.publish((ZIEL3.c_str()),msg);
   if (Pos > maxPos) maxPos = Pos;
   dtostrf(maxPos ,3,0, msg);
   client.publish((ZIEL5.c_str()),msg);
   if (maxPos < 1) maxPos = 1;
   int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
   if (P_Pos > 100) P_Pos = 100;
   dtostrf(P_Pos ,3,0, msg);
   client.publish((ZIEL4.c_str()),msg);
   //Serial.print("Position in %   : ");
   //Serial.println(P_Pos);
   //Serial.print("Position absolut: ");
   //Serial.println(Pos);
   //Serial.print("Maximal Position: ");
   //Serial.println(maxPos);
   prevSENSOR1 = sensor1;
   doSENSOR1 = 0;
  }
    
//------ PIN ist LOW während der Rollo öffnet ---------------- 

  int sensor2 = digitalRead(SENSOR2);               
  if (sensor2 == 0)
   {
    set_Down = 0;
    set_Up = 1;
   }
  if (doSENSOR2 || sensor2 != prevSENSOR2)
  {
   const char *state = (sensor2) ? "Stop" : "Open";
   client.publish((ZIEL2.c_str()),msg);
   //Serial.print("...sende an ");
   //Serial.print(ZIEL2);
   //Serial.print(" :  ");
   //Serial.println(state);
   dtostrf(Pos ,3,0, msg);
   client.publish((ZIEL3.c_str()),msg);
   if (Pos > maxPos) maxPos = Pos;
   dtostrf(maxPos ,3,0, msg);
   client.publish((ZIEL5.c_str()),msg);
   if (maxPos < 1) maxPos = 1;
   int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
   if (P_Pos > 100) P_Pos = 100;
   dtostrf(P_Pos ,3,0, msg);
   client.publish((ZIEL4.c_str()),msg);
   //Serial.print("Position in %   : ");
   //Serial.println(P_Pos);
   //Serial.print("Position absolut: ");
   //Serial.println(Pos);
   //Serial.print("Maximal Position: ");
   //Serial.println(maxPos);
   prevSENSOR2 = sensor2;
   doSENSOR2 = 0;
  } 
    
//------ PIN ist Impulseingang des REED-Kontakt`s des GW 60 ----------------- 

  int sensor3 = digitalRead(SENSOR3);
  if (doSENSOR3 || sensor3 != prevSENSOR3)
  {
   if ((prevSENSOR3 == HIGH) && (sensor3 == LOW))
    {
     if ((sensor3 == LOW) && (sensor1 == LOW))
      {
       Pos = (Pos - step);
       if (Pos <= 0) Pos = 0;
      }
     if ((sensor3 == LOW) && (sensor2 == LOW)) 
      {
       Pos = (Pos + step);
      }
    } 
    dtostrf(Pos ,3,0, msg);
    client.publish((ZIEL3.c_str()),msg);
    if (maxPos < 1) maxPos = 1;
    int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
    if (P_Pos > 100) P_Pos = 100;
    dtostrf(P_Pos ,3,0, msg);
    client.publish((ZIEL4.c_str()),msg);
    if (Pos > maxPos) maxPos = Pos;
    dtostrf(maxPos ,3,0, msg);
    client.publish((ZIEL5.c_str()),msg);
    //Serial.print("Position in %   : ");
    //Serial.println(P_Pos);
    //Serial.print("Position absolut: ");
    //Serial.println(Pos);
    //Serial.print("Maximal Position: ");
    //Serial.println(maxPos);
    EEPROM.begin(2);
    EEPROM.write(0, Pos);
    EEPROM.write(1, maxPos);
    EEPROM.end();
    prevSENSOR3 = sensor3;
    doSENSOR3 = 0;
  }
  
  //-----PIN ist LOW zu Speichern---------------------
  
  int sensor4 = digitalRead(SENSOR4);
  if (doSENSOR4 || sensor4 != prevSENSOR4)
   {
    if ((prevSENSOR4 == HIGH) && (sensor4 == LOW))
     {
      if ((sensor4 == LOW) && (sensor1 == HIGH) && (set_Down == 1))
       {
        Pos = 0;
        dtostrf(Pos ,3,0, msg);
        client.publish((ZIEL3.c_str()),msg);
        int P_Pos = 0;
        dtostrf(P_Pos ,3,0, msg);
        client.publish((ZIEL4.c_str()),msg);
        //maxPos = 0;
        dtostrf(maxPos ,3,0, msg);
        client.publish((ZIEL5.c_str()),msg);
        set_Down = 0;
        set_Up = 0 ;
        set_TDown = 0;
        preset_TDown = 0;
        set_TUp = 0;
        preset_TUp = 0;
       }
      if ((sensor4 == LOW) && (sensor2 == HIGH) && (set_Up == 1))
       {
        maxPos = Pos;
        if (maxPos < 1) maxPos =1;
        int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
        if (P_Pos > 100) P_Pos = 100;
        dtostrf(P_Pos ,3,0, msg);
        client.publish((ZIEL4.c_str()),msg);
        if (Pos > maxPos) maxPos = Pos;
        dtostrf(maxPos ,3,0, msg);
        client.publish((ZIEL5.c_str()),msg);
        EEPROM.begin(2);
        EEPROM.write(0, Pos);
        EEPROM.write(1, maxPos);
        EEPROM.end();
        set_Down = 0;
        set_Up = 0 ;
        set_TDown = 0;
        preset_TDown = 0;
        set_TUp = 0;
        preset_TUp = 0;
       }
     }
      prevSENSOR4 = sensor4;
      doSENSOR4 = 0;
     } 
     
     //-----Autoabgleich und Speichern: Wird der Rollo über z.B. FHEM angesteuert ud durch die programmierten Endeinstellngen des GW 60 gestoppt,
     //                                 wird der 0 und Maximalwert auomatisch gspeichert !!!!
     
     if ((sensor1 == HIGH) && (set_TDown == 1))
      {
       Pos = 0;
       dtostrf(Pos ,3,0, msg);
       client.publish((ZIEL3.c_str()),msg);
       int P_Pos = 0;
       dtostrf(P_Pos ,3,0, msg);
       client.publish((ZIEL4.c_str()),msg);
       //maxPos = 0;
       dtostrf(maxPos ,3,0, msg);
       client.publish((ZIEL5.c_str()),msg);
       set_Down = 0;
       set_Up = 0 ;
       set_TDown = 0;
       preset_TDown = 0;
       set_TUp = 0;
       preset_TUp = 0;
      }
     if ((sensor2 == HIGH) && (set_TUp == 1))
      {
       pre_maxPos = maxPos;
       maxPos = Pos;
       if (maxPos < 1) maxPos = 1;
       int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
       if (P_Pos > 100) P_Pos = 100;
       dtostrf(P_Pos ,3,0, msg);
       client.publish((ZIEL4.c_str()),msg);
       if (Pos > maxPos) maxPos = Pos;
       dtostrf(maxPos ,3,0, msg);
       client.publish((ZIEL5.c_str()),msg);
       EEPROM.begin(2);
       EEPROM.write(0, Pos);
       EEPROM.write(1, maxPos);
       EEPROM.end();
       set_Down = 0;
       set_Up = 0 ;
       set_TDown = 0;
       preset_TDown = 0;
       set_TUp = 0;
       preset_TUp = 0;
      }
} 
   


