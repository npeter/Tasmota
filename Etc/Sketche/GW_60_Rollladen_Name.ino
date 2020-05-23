//GW_60_Rollladen_Name.ino

// IP - Adressen

int IP1    = 192;    // 1. Stelle IP - Adresse
int IP2    = 178;    // 2. Stelle IP - Adresse
int IP3    = 0;      // 3. Stelle IP - Adresse
int GWY    = 1;      // 4. Stelle IP - Adresse Gateway
int WIP    = 140;    // 4. Stelle IP - Adresse Wunsch-IP --> ist WIP = 0 dann ist DHCP aktiv
int DN1    = 255;    // 1. Stelle DNS
int DN2    = 255;    // 2. Stelle DNS
int DN3    = 255;    // 3. Stelle DNS
int DN4    = 0;      // 4. Stelle DNS


// Const char* für eigene Bedürfnisse anpassen !!!!!!

const char* mqtt_server = "192.178.0.XXX"; // XXX --> IP auf der FHEM und Mosquitto läuft

const char* ssid        = "Deine SSID";
const char* password    = "Dein WLAN Passwort";
const char* clientName  = "EG_Büro";
const char* start       = "Pi";
const char* QUELLE1     = "EG_Buero/Down";          // Ausgang --> Taste Down
const char* QUELLE2     = "EG_Buero/Up";            // Ausgang --> Taste Up
const char* ZIEL1       = "EG_Buero/Schliessen";    // Eingang --> Rollo schließt sich
const char* ZIEL2       = "EG_Buero/Oeffnen";       // Eingang --> Rollo öffnet sich
const char* ZIEL3       = "EG_Buero/Position";      // Eingang --> Rolloposition 
const char* ZIEL4       = "EG_Buero/Position_%";
const char* ZIEL5       = "EG_Buero/maxPosition";



// ALife-Intervall in Sekunden für die Sensoren, wenn keine Abfrage erfolgt

int Time_SENSOR      = 60;     


//
// ENDE der individuellen Einstellungen !!!!!
//

// Ab hier nur Änderungen bezüglich anderer PIN-Zuweisung nötig !!!!!

#define SENSOR1       0   // D3   mit Pullup ; Aktive-Low; TASTE DOWN
#define SENSOR2       2   // D4   mit Pullup ; Aktive-Low; TASTE UP
#define SENSOR3      16   // D0   mit Pullup ; Aktive-Low; REED KONTAKT
#define SENSOR4      14   // D5   mit Pullup ; Aktive-Low; TASTE STORAGE
#define AKTOR1        4   // D2   LOW-Aktiv              ; MOTOR -> ZU
#define AKTOR2        5   // D1   LOW-Aktiv              ; MOTOR -> AUF 


// Folgende Zeilen müssen normal nicht geändert werden !!!!!!!

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

int ON_Zeit          = 1000;   //ON-Zeit der Aktoren in ms
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
 if (WIP > 0) WiFi.config(IPAddress(IP1,IP2,IP3,WIP), IPAddress(IP1,IP2,IP3,GWY), IPAddress(DN1,DN2,DN3,DN4), IPAddress(IP1,IP2,IP3,GWY)); //Feste IP-Vergabe
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
  if (client.connect(clientName))
  {
   //Serial.print("");
   //Serial.print("...empfange Anmeldung erfolgreich an :  ");
   client.publish(start, clientName);
   //Serial.print(start);
   //Serial.print("  ");
   //Serial.println(clientName);
   //Serial.print(" ");
   //Serial.println(" ");
   //Serial.print("");
   client.subscribe(QUELLE1);
   client.subscribe(QUELLE2);
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
   client.publish(ZIEL1,(state));
   //Serial.print("...sende an ");
   //Serial.print(ZIEL1);
   //Serial.print(" :  ");
   //Serial.println(state);
   dtostrf(Pos ,3,0, msg);
   client.publish(ZIEL3,msg);
   if (Pos > maxPos) maxPos = Pos;
   dtostrf(maxPos ,3,0, msg);
   client.publish(ZIEL5,msg);
   if (maxPos < 1) maxPos = 1;
   int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
   if (P_Pos > 100) P_Pos = 100;
   dtostrf(P_Pos ,3,0, msg);
   client.publish(ZIEL4,msg);
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
   client.publish(ZIEL2,(state));
   //Serial.print("...sende an ");
   //Serial.print(ZIEL2);
   //Serial.print(" :  ");
   //Serial.println(state);
   dtostrf(Pos ,3,0, msg);
   client.publish(ZIEL3,msg);
   if (Pos > maxPos) maxPos = Pos;
   dtostrf(maxPos ,3,0, msg);
   client.publish(ZIEL5,msg);
   if (maxPos < 1) maxPos = 1;
   int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
   if (P_Pos > 100) P_Pos = 100;
   dtostrf(P_Pos ,3,0, msg);
   client.publish(ZIEL4,msg);
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
    client.publish(ZIEL3,msg);
    if (maxPos < 1) maxPos = 1;
    int P_Pos = (Pos * 100 / maxPos);    // absolut auf % umrechnen
    if (P_Pos > 100) P_Pos = 100;
    dtostrf(P_Pos ,3,0, msg);
    client.publish(ZIEL4,msg);
    if (Pos > maxPos) maxPos = Pos;
    dtostrf(maxPos ,3,0, msg);
    client.publish(ZIEL5,msg);
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
        client.publish(ZIEL3,msg);
        int P_Pos = 0;
        dtostrf(P_Pos ,3,0, msg);
        client.publish(ZIEL4,msg);
        //maxPos = 0;
        dtostrf(maxPos ,3,0, msg);
        client.publish(ZIEL5,msg);
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
        client.publish(ZIEL4,msg);
        if (Pos > maxPos) maxPos = Pos;
        dtostrf(maxPos ,3,0, msg);
        client.publish(ZIEL5,msg);
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
       client.publish(ZIEL3,msg);
       int P_Pos = 0;
       dtostrf(P_Pos ,3,0, msg);
       client.publish(ZIEL4,msg);
       //maxPos = 0;
       dtostrf(maxPos ,3,0, msg);
       client.publish(ZIEL5,msg);
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
       client.publish(ZIEL4,msg);
       if (Pos > maxPos) maxPos = Pos;
       dtostrf(maxPos ,3,0, msg);
       client.publish(ZIEL5,msg);
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
   


