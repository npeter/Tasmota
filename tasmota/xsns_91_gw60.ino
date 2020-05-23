/*
  xsns_91_gw60.ino - GW60 - Smart Superrollo for Sonoff-Tasmota

  Copyright (C) 2019 Peter Neeser

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
  * This is a part (device firmware) of a DIY project to integrate smart HW/FW extension of 
  * GW60 SuperRollo into sonof-tasmota environment and iobroker
  * http://www.superrollo-online.de/elektronische-rollladengurtwickler/superrollo-gw60/
  *
  * The HW ideas are based on the work from Björn Hempel, Uwe ToDo, others ... 
  * https://www.hempel-online.de/cms/index.php/fhem/articles/umbau-superrollo-gw60-fuer-homematic.html
  * 
  * The standard device GW60 is extended by a smart HW/FW which enables a remote control of the blind.
  * It's just an extensions. All original GW60 functions still avaliable. 
  * It offers an additional interface via home automation systems. This project was develo and tested 
  * with iobroker https://www.iobroker.net/

  * New smart functions:
  *   state: blind position (0%..100%); motor status
  *   cmd:  position blind to x% (0%-blind open; 100%->blind closed)
  *         up/down buttom 
  *   tele: Temperatur sensor DS18x20
  * 
  * The HW is based on a WEMOS D1 mini. The FW is sonof-tasmota 6.5.x with a propreitary GW60 driver 
  * (xsns_911_gw60.ino, ...). 
  * I developed a PCB, a housing and extended sonof-tasmota FW. Details see todo
  * 
  * Peter Neeser Rev. 2019-06-25
  *                   2020-01-26 migration to Tasmota 8.1
 */

#ifdef USE_GW60

#define XSNS_91 91


#define D_LOG_GW60 "GW6: "
#define D_XSNS_GW60_INIT "XsnsGw60Init() "
#define D_XSNS_GW60_SHOW_JSON "XsnsGw60Show(Json=%d) "
#define D_XSNS_GW60_EVERY_SECOND "XsnsGw60EverySecond() "
#define D_XSNS_GW60_COMMAND "XsnsGw60Command() "
#define D_XSNS_GW60_HANDLER "XsnsGw60Handler() "


#define GW60_MAX_SCOUNTER 500      // max / min value for max_scounter check
#define GW60_MAX_SCOUNTER_MIN 40   //
#define GW60_INACURRACY_SCOUNTER 5 // Window for full up/down correction

/*
  Values for POS:
    Range: from GW60_MIN_VALUE_POS to GW60_MAX_VALUE_POS
           without any gap
 */
#define GW60_MAX_POS 100 // blind down POS
#define GW60_MIN_POS 0   // blind up POS
#define GW60_AUTO_HOMING_POS -1
#define GW60_CLEAR_HOMING_POS -2
#define GW60_MANUAL_HOMING_POS -3
#define GW60_MAX_VALUE_POS GW60_MAX_POS
#define GW60_MIN_VALUE_POS GW60_MANUAL_HOMING_POS

#define D_GW60 "GWx"
#define D_GW60_FOUND "GW60 cfg found"
#define D_GW60_TRUE "true"
#define D_GW60_FALSE "false"

/*----------------------------------------------------------------------*
 Local data and types
  ----------------------------------------------------------------------*/
const char S_LOG_GW60[] PROGMEM = D_LOG_GW60;

enum Gw60States
{
  GW60_SM_INIT,
  GW60_SM_WAIT_4_REFPOS,
  GW60_SM_CHECK_STOP,
  GW60_SM_START_MOVE,
  GW60_SM_CHECK_UP_DOWN_MOVE,
  GW60_SM_CHECK_POS_MOVE_UP,
  GW60_SM_CHECK_POS_MOVE_DOWN,
  GW60_SM_CHECK_POS_STOP,
} gw60_sm = GW60_SM_INIT;

//
bool gw60_ok = false; // correct gw60 configuration available

// homing control flags
bool gw60_homing_up = false;     // true: UP homing done
bool gw60_homing = false;        // true: UP and DOWN homing done
bool gw60_auto_homing = false;   // true: auto homing procedure activated
bool gw60_up_after_init = false; // true: move UP after init activated

// incremental blind positions (hall sensor events)
int16_t gw60_scounter = 0;                     // current incremental blind position 0,...,gw60_max_scounter (0 / 0% / up)
int16_t gw60_max_scounter = GW60_MAX_SCOUNTER; // max. incremental blind position (100% / down)
int16_t gw60_ref_scounter = 0;                 // incremental blind position 0,...,gw60_max_scounter - reference value

// pos and refpos as relative blind positions 0,....,100% (up,...,down)
int16_t gw60_pos = 0;          // current relative blind position
bool gw60_new_ref_pos = false; // true: New gw60_ref_pos received by MQTT command
int16_t gw60_ref_pos = 0;      // relative blind position reference value received by MQTT command
int16_t gw60_ref_pos_sm = 0;   // relative blind position tmp buffer for state machine

// buffers for last published values via MQTT tele
bool gw60_tele_homing_up = false;
bool gw60_tele_homing = false;
int16_t gw60_tele_pos = 0;
int16_t gw60_tele_ref_pos = 0;
bool gw60_tele_m1 = true;
bool gw60_tele_m2 = true;
bool gw60_tele_up = false;
bool gw60_tele_down = false;
int16_t gw60_tele_scounter = 0;
Gw60States gw60_tele_sm = GW60_SM_INIT;

/*----------------------------------------------------------------------*
| locate ISR in IRAM
  ----------------------------------------------------------------------*/
void ICACHE_RAM_ATTR XsnsGw60Interrupt(void); // pn 2020-01-27

/*----------------------------------------------------------------------*
  Gw60 utility(s)
  ----------------------------------------------------------------------*/
static int Gw60Count2Pos()
{
  int pos = (gw60_scounter * GW60_MAX_POS + (gw60_max_scounter >> 1)) / gw60_max_scounter;
  return ((pos > GW60_MAX_POS) ? GW60_MAX_POS : pos);
}

/*----------------------------------------------------------------------*
  LogSm 
  ----------------------------------------------------------------------*/
static void LogSm(int old_state, int new_state)
{
  AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "state changed: %d -> %d"), old_state, new_state);
}

/*----------------------------------------------------------------------*
  Debugging only - disable for release version!
  ----------------------------------------------------------------------*/
//#define GW60_DEBUG_HEAP

#ifdef GW60_DEBUG_HEAP

uint32_t heap_curr = 0;
uint32_t heap_last = 0;
uint32_t heap_used = 0;
uint32_t heap_changed = 0;

static void Gw60ShowHeap()
{
  if ((uptime % 10) == 0)
  {
    heap_curr = ESP.getFreeHeap();
    if (heap_last)
    {
      if (heap_last != heap_curr)
      {
        heap_used += (heap_last - heap_curr);
        heap_last = heap_curr;
        heap_changed++;
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR("HEAP=%d used=%d changed=%d"), heap_curr, heap_used, heap_changed);
      }
    }
    else
    {
      heap_last = heap_curr;
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR("HEAP=%d used=%d"), heap_curr, heap_used);
    }
  }
}
#endif

/*----------------------------------------------------------------------*
  XsnsGw60Init / Callback ID: FUNC_INIT
  ----------------------------------------------------------------------*/
static bool XsnsGw60Init()
{
  AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 D_XSNS_GW60_INIT));

  gw60_ok = false;

  // if GW60 is configured
  if ((Pin(GPIO_GW60_MOTOR_1) < 99) && (Pin(GPIO_GW60_MOTOR_2) < 99) && (Pin(GPIO_GW60_HALL_S) < 99) &&
      (Pin(GPIO_GW60_UP) < 99) && (Pin(GPIO_GW60_DOWN)))
  {

    AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 D_GW60_FOUND));

    // init GPIO
    pinMode(Pin(GPIO_GW60_MOTOR_1), INPUT_PULLUP);
    pinMode(Pin(GPIO_GW60_MOTOR_2), INPUT_PULLUP);
    pinMode(Pin(GPIO_GW60_HALL_S), INPUT_PULLUP);
    pinMode(Pin(GPIO_GW60_UP), OUTPUT);
    pinMode(Pin(GPIO_GW60_DOWN), OUTPUT);

    digitalWrite(Pin(GPIO_GW60_UP), true);
    digitalWrite(Pin(GPIO_GW60_DOWN), true);

    // get and test settings
    gw60_scounter = 0;
    gw60_max_scounter = Settings.gw60_max_scounter;

    if ((gw60_max_scounter > GW60_MAX_SCOUNTER_MIN) && (gw60_max_scounter < GW60_MAX_SCOUNTER))
    {                            // valid settings
      gw60_up_after_init = true; // move up to 0 position
      gw60_homing_up = false;    // 0 position not known
      gw60_homing = true;        // scaling known
    }
    else // invalid settings
    {
      gw60_up_after_init = false; // avoid unexpected move during mouting
      gw60_homing_up = false;     // 0 position not known
      gw60_homing = false;        // scaling unknown
      gw60_max_scounter = GW60_MAX_SCOUNTER;
    }

    AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 D_XSNS_GW60_INIT "Settings.msc=%d msc=%d"), Settings.gw60_max_scounter, gw60_max_scounter);
    //Settings.gw60_max_scounter = gw60_max_scounter;

    gw60_ok = true;
    attachInterrupt(Pin(GPIO_GW60_HALL_S), XsnsGw60Interrupt, CHANGE);
  }

  return gw60_ok;
}

const char HTTP_GW60_SHOW[] PROGMEM = "M1=%s M2=%s SC=%d MSC=%d POS=%i REFPOS=%i UP=%s DOWN=%s";

/*----------------------------------------------------------------------*
  XsnsGw60Show / Callback ID: FUNC_JSON_APPEND and FUNC_WEB_APPEND
  ----------------------------------------------------------------------*/
static bool XsnsGw60Show(bool Json)
{
  AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_SHOW_JSON), Json);

  bool result = false;

  gw60_pos = Gw60Count2Pos();

  if (Json)
  {
    snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("%s,\"M1\":\"%s\",\"M2\":\"%s\",\"SC\":%i,\"MSC\":%i,\"POS\":%i,\"REFPOS\":%i,\"UP\":\"%s\",\"DOWN\":\"%s\""),
               mqtt_data,
               digitalRead(Pin(GPIO_GW60_MOTOR_1)) ? D_GW60_FALSE : D_GW60_TRUE,
               digitalRead(Pin(GPIO_GW60_MOTOR_2)) ? D_GW60_FALSE : D_GW60_TRUE,
               gw60_scounter, gw60_max_scounter, gw60_pos, gw60_ref_pos,
               digitalRead(Pin(GPIO_GW60_UP)) ? D_GW60_FALSE : D_GW60_TRUE,
               digitalRead(Pin(GPIO_GW60_DOWN)) ? D_GW60_FALSE : D_GW60_TRUE);
  }

#ifdef USE_WEBSERVER
  else
  {
    // Migration to V8.1 - adapt to new WS API
    WSContentSend_P(HTTP_GW60_SHOW,
                    digitalRead(Pin(GPIO_GW60_MOTOR_1)) ? D_GW60_FALSE : D_GW60_TRUE,
                    digitalRead(Pin(GPIO_GW60_MOTOR_2)) ? D_GW60_FALSE : D_GW60_TRUE,
                    gw60_scounter, gw60_max_scounter, gw60_pos, gw60_ref_pos,
                    digitalRead(Pin(GPIO_GW60_UP)) ? D_GW60_FALSE : D_GW60_TRUE,
                    digitalRead(Pin(GPIO_GW60_DOWN)) ? D_GW60_FALSE : D_GW60_TRUE);
  }
#endif // USE_WEBSERVER

  return result;
}

/*----------------------------------------------------------------------*
  XsnsGw60Command / Callback ID: FUNC_COMMAND
  ----------------------------------------------------------------------*/
static bool XsnsGw60Command()
{
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 D_XSNS_GW60_COMMAND));
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- grpflg=%d"), XdrvMailbox.grpflg);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- usridx=%d"), XdrvMailbox.usridx);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- command_code=%d"), XdrvMailbox.command_code);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- index=%d"), XdrvMailbox.index);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- data_len=%d"), XdrvMailbox.data_len);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- payload=%d"), XdrvMailbox.payload);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- topic=%s"), XdrvMailbox.topic);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- data=%s"), XdrvMailbox.data);
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "- command=%s"), XdrvMailbox.command);

  if (strncmp(XdrvMailbox.topic, "REFPOS", 6) == 0)
  {
    if (XdrvMailbox.data_len > 0)
    {
      if (!gw60_new_ref_pos)
      {
        if ((XdrvMailbox.payload >= GW60_MIN_VALUE_POS) && (XdrvMailbox.payload <= GW60_MAX_VALUE_POS))
        {
          gw60_ref_pos = XdrvMailbox.payload;
          gw60_new_ref_pos = true;
          snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("{\"REFPOS\":%i}"), gw60_ref_pos);
          MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);
        }
        else
        {
          AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 "parameter invalid %d"), XdrvMailbox.payload);
        }
      }
      else
      {
        AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 "GW60 busy"));
      }
    }
    else
    {
      AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 "parameter missed"));
    }
  }

  else if (strncmp(XdrvMailbox.topic, "DOWN", 4) == 0)
  {
    if (strncmp(XdrvMailbox.topic, "ON", 2) == 0)
    {
      digitalWrite(Pin(GPIO_GW60_DOWN), false);
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("{\"DOWN\":%s}"), digitalRead(Pin(GPIO_GW60_DOWN)) ? "OFF" : "ON");
      MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);
    }
    else if (strncmp(XdrvMailbox.topic, "OFF", 3) == 0)
    {
      digitalWrite(Pin(GPIO_GW60_DOWN), true);
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("{\"DOWN\":%s}"), digitalRead(Pin(GPIO_GW60_DOWN)) ? "OFF" : "ON");
      MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);
    }
    else
    {
      AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 "parameter invalid %s"), XdrvMailbox.data);
    }
  }

  else if (strncmp(XdrvMailbox.topic, "UP", 2) == 0)
  {
    if (strncmp(XdrvMailbox.topic, "ON", 2) == 0)
    {
      digitalWrite(Pin(GPIO_GW60_UP), false);
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("{\"UP\":%s}"), digitalRead(Pin(GPIO_GW60_UP)) ? "OFF" : "ON");
      MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);
    }
    else if (strncmp(XdrvMailbox.topic, "OFF", 3) == 0)
    {
      digitalWrite(Pin(GPIO_GW60_UP), true);
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("{\"UP\":%s}"), digitalRead(Pin(GPIO_GW60_UP)) ? "OFF" : "ON");
      MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);
    }
    else
    {
      AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 "parameter invalid %s"), XdrvMailbox.data);
    }
  }

  else
  {
    AddLog_P2(LOG_LEVEL_INFO, PSTR(D_LOG_GW60 "invalid topic %s"), XdrvMailbox.topic);
  }

  return true;
}

/*----------------------------------------------------------------------*
  Gw60EverySecond / Callback ID: FUNC_EVERY_SECOND
  ----------------------------------------------------------------------*/
bool XsnsGw60EverySecond()
{
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "_XSNS_GW60_EVERY_SECOND"));

  bool result = false;

  if (MqttIsConnected())
  {
    // Mqtt part 1 -> M1 M2 M UP DOWN SM --------------------------------------
    static bool firstPublischStates = true;

    bool m1 = digitalRead(Pin(GPIO_GW60_MOTOR_1));
    bool m2 = digitalRead(Pin(GPIO_GW60_MOTOR_2));
    bool up = digitalRead(Pin(GPIO_GW60_UP));
    bool down = digitalRead(Pin(GPIO_GW60_DOWN));
    Gw60States sm = gw60_sm;

    if (firstPublischStates or
        (m1 != gw60_tele_m1) or (m2 != gw60_tele_m2) or (up != gw60_tele_up) or (down != gw60_tele_down) or (sm != gw60_tele_sm))
    {
      snprintf_P(mqtt_data, sizeof(mqtt_data),
                 PSTR("{\"M1\":\"%s\",\"M2\":\"%s\",\"M\":\"%s\",\"UP\":\"%s\",\"DOWN\":\"%s\",\"SM\":%i}"),
                 (m1 ? D_GW60_FALSE : D_GW60_TRUE),
                 (m2 ? D_GW60_FALSE : D_GW60_TRUE),
                 ((m1 && m2) ? D_GW60_FALSE : D_GW60_TRUE),
                 (up ? D_GW60_FALSE : D_GW60_TRUE),
                 (down ? D_GW60_FALSE : D_GW60_TRUE),
                 sm);
      MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);

      gw60_tele_m1 = m1;
      gw60_tele_m2 = m2;
      gw60_tele_up = up;
      gw60_tele_down = down;
      gw60_tele_sm = sm;

      firstPublischStates = false;
    }

    // Mqtt part 2 -> SC POS REFPOS ------------------------------------------------------
    static bool firstPublischPos = true;

    int scounter = gw60_scounter;
    int pos = gw60_pos = Gw60Count2Pos();
    int ref_pos = gw60_ref_pos;

    if (firstPublischPos or (scounter != gw60_tele_scounter) or (pos != gw60_tele_pos) or (ref_pos != gw60_tele_ref_pos))
    {
      snprintf_P(mqtt_data, sizeof(mqtt_data),
                 PSTR("{\"SC\":%i,\"POS\":%i,\"REFPOS\":%i}"),
                 scounter, pos, ref_pos);
      MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);

      gw60_tele_scounter = scounter;
      gw60_tele_pos = pos;
      gw60_tele_ref_pos = ref_pos;

      firstPublischPos = false;
    }

    // Mqtt part 3 -> HOME_UP HOME MSC ----------------------------------------
    static bool firstPublishHoming = true;

    bool homing_up = gw60_homing_up;
    bool homing = gw60_homing;

    if (firstPublishHoming or (homing_up != gw60_tele_homing_up) or (homing != gw60_tele_homing))
    {
      snprintf_P(mqtt_data, sizeof(mqtt_data), PSTR("{\"HOME_UP\":\"%s\",\"HOME\":\"%s\",\"MSC\":%d}"),
                 (homing_up ? D_GW60_TRUE : D_GW60_FALSE),
                 (homing ? D_GW60_TRUE : D_GW60_FALSE),
                 gw60_max_scounter);
      MqttPublishPrefixTopic_P(RESULT_OR_STAT, mqtt_data);

      gw60_tele_homing_up = homing_up;
      gw60_tele_homing = homing;

      firstPublishHoming = false;
    }

#ifdef GW60_DEBUG_HEAP
    Gw60ShowHeap();
#endif
  }

  return result;
}

/*----------------------------------------------------------------------*
  XsnsGW60Handler
  ----------------------------------------------------------------------*/
/* state model

	                 |INIT |WAIT_4_REFPOS	   |CHECK_STOP	       |START_MOVE	       |CHECK_UP_DOWN_MOVE |CHECK_POS_MOVE_UP	 |CHECK_POS_MOVE_DOWN|CHECK_POS_STOP
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
INIT	             |na	 |always	         |na	               |na	               |na	               |na	               |na	               |na
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
WAIT_4_REFPOS	     |na   |AUTO_HOMING_POS  |                   |                   |                   |                   |                   |
                   |     |RESET_HOMING_POS |                   |                   |                   |                   |                   |
                   |     |MANUAL_HOMING_POS|                   |                   |                   |                   |                   |
                   |     |invalid REFPOS   |                   |                   |                   |                   |                   |
                   |     |cyclic	         |na	               |move not possible	 |M1 && M2 (stopped) |M1 && M2 (stopped) |M1 && M2 (stopped) |M1 && M2 (stopped)
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
CHECK_STOP	       |na	 |MIN>=REFPOS<=MAX |cyclic	           |na	               |na	               |na	               |na	               |na
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
START_MOVE	       |na	 |na	             |M1 && M2 (stopped) |na	               |na	               |na	               |na	               |na
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
CHECK_UP_DOWN_MOVE |na	 |na	             |na                 |REFPOS = MIN (UP)  |                   |                   |                   | 
                   |     |                 |                   |REFPOS = MAX (DOWN)|cyclic	           |na	               |na	               |na
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
CHECK_POS_MOVE_UP	 |na	 |na	             |na	               |pos up	           |na	               |cyclic	           |na	               |na
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
CHECK_POS_MOVE_DOWN|na	 |na	             |na	               |pos down	         |na	               |na	               |cyclic	           |na
-------------------+-----+-----------------+-------------------+-------------------+-------------------+-------------------+-------------------+--------------------
CHECK_POS_STOP	   |na	 |na	             |na	               |na	               |na	               |POS ~ REFPOS	     |POS ~ REFPOS	     |cyclic

na - not aplicable

*/
static bool XsnsGW60Handler()
{
  AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER));

  Gw60States old_gw60_sm = gw60_sm;

  switch (gw60_sm)
  {

  // 0 GW60_SM_INIT ===========================================================
  case GW60_SM_INIT:
    if (gw60_up_after_init)
    {
      gw60_new_ref_pos = true;
      gw60_ref_pos = 0;
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "up after init"), gw60_ref_pos);
    }
    else
    {
      gw60_new_ref_pos = false;
    }

    gw60_sm = GW60_SM_WAIT_4_REFPOS;
    XsnsGw60EverySecond();
    LogSm(old_gw60_sm, gw60_sm);
    break;

  // 1 GW60_SM_WAIT_4_REFPOS ==================================================
  case GW60_SM_WAIT_4_REFPOS:
    if (gw60_new_ref_pos)
    {
      gw60_ref_pos_sm = gw60_ref_pos;

      // UP,DOWN, POS ---------------------------------------------------------
      if ((gw60_ref_pos_sm >= GW60_MIN_POS) && (gw60_ref_pos_sm <= GW60_MAX_POS))
      {
        if (!digitalRead(Pin(GPIO_GW60_MOTOR_1)) || !digitalRead(Pin(GPIO_GW60_MOTOR_2)))
        {
          digitalWrite(Pin(GPIO_GW60_UP), false);
          digitalWrite(Pin(GPIO_GW60_DOWN), false);
        }
        gw60_sm = GW60_SM_CHECK_STOP;
        XsnsGw60EverySecond();
        LogSm(old_gw60_sm, gw60_sm);
      }

      // auto homing-----------------------------------------------------------
      else if (gw60_ref_pos_sm == GW60_AUTO_HOMING_POS)
      {
        gw60_homing_up = false;
        gw60_homing = false;
        Settings.gw60_max_scounter = gw60_max_scounter = GW60_MAX_SCOUNTER;

        gw60_auto_homing = true;
        gw60_ref_pos = GW60_MIN_POS;

        gw60_sm = GW60_SM_WAIT_4_REFPOS;
        XsnsGw60EverySecond();
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "auto homing REFPOS=%d"), gw60_ref_pos);
        LogSm(old_gw60_sm, gw60_sm);
      }

      // reset homing ---------------------------------------------------------
      else if (gw60_ref_pos_sm == GW60_CLEAR_HOMING_POS)
      {
        gw60_homing_up = false;
        gw60_homing = false;
        Settings.gw60_max_scounter = gw60_max_scounter = GW60_MAX_SCOUNTER;

        gw60_new_ref_pos = false;
        gw60_sm = GW60_SM_WAIT_4_REFPOS;
        XsnsGw60EverySecond();
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "clear homing msc=%d"), gw60_max_scounter);
        LogSm(old_gw60_sm, gw60_sm);
      }

      // manual homing --------------------------------------------------------
      else if (gw60_ref_pos_sm == GW60_MANUAL_HOMING_POS)
      {
        Settings.gw60_max_scounter = gw60_max_scounter = (gw60_scounter > 0) ? gw60_scounter : GW60_MAX_SCOUNTER; // avoid div by 0
        gw60_homing_up = true;
        gw60_homing = true;
        gw60_auto_homing = false;

        gw60_new_ref_pos = false;
        gw60_sm = GW60_SM_WAIT_4_REFPOS;
        XsnsGw60EverySecond();
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "manual homing mc=%d"), gw60_max_scounter);
        LogSm(old_gw60_sm, gw60_sm);
      }

      else // invalid parameter -----------------------------------------------
      {
        gw60_new_ref_pos = false;
        gw60_sm = GW60_SM_WAIT_4_REFPOS;
        XsnsGw60EverySecond();
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "invalid POS %d"), gw60_ref_pos_sm);
        LogSm(old_gw60_sm, gw60_sm);
      }
    }

    else // cyclic task
      gw60_ref_pos = gw60_pos;

    break;

  // 2 GW60_SM_CHECK_STOP =============================================
  case GW60_SM_CHECK_STOP:
    if (digitalRead(Pin(GPIO_GW60_MOTOR_1)) && digitalRead(Pin(GPIO_GW60_MOTOR_2)))
    { // motors stopped
      digitalWrite(Pin(GPIO_GW60_UP), true);
      digitalWrite(Pin(GPIO_GW60_DOWN), true);
      gw60_sm = GW60_SM_START_MOVE;
      XsnsGw60EverySecond();
      LogSm(old_gw60_sm, gw60_sm);
    }
    break;

  // 3 GW60_SM_START_MOVE =============================================
  case GW60_SM_START_MOVE:
    if (gw60_ref_pos_sm == GW60_MIN_POS) // move up ------------
    {
      digitalWrite(Pin(GPIO_GW60_UP), false);
      gw60_sm = GW60_SM_CHECK_UP_DOWN_MOVE;
      XsnsGw60EverySecond();
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "up"));
      LogSm(old_gw60_sm, gw60_sm);
    }
    else if (gw60_ref_pos_sm == GW60_MAX_POS) // move down ----------
    {
      digitalWrite(Pin(GPIO_GW60_DOWN), false);
      gw60_sm = GW60_SM_CHECK_UP_DOWN_MOVE;
      XsnsGw60EverySecond();
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "down"));
      LogSm(old_gw60_sm, gw60_sm);
    }
    else if (gw60_homing)
    {
      gw60_ref_scounter = gw60_ref_pos_sm * gw60_max_scounter / GW60_MAX_POS;
      if (gw60_ref_scounter < gw60_scounter) // pos up -------------
      {
        digitalWrite(Pin(GPIO_GW60_UP), false);
        gw60_sm = GW60_SM_CHECK_POS_MOVE_UP;
        XsnsGw60EverySecond();
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "move up"));
        LogSm(old_gw60_sm, gw60_sm);
      }
      else if (gw60_ref_scounter > gw60_scounter) // pos down -----------
      {
        digitalWrite(Pin(GPIO_GW60_DOWN), false);
        gw60_sm = GW60_SM_CHECK_POS_MOVE_DOWN;
        XsnsGw60EverySecond();
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "move down"));
        LogSm(old_gw60_sm, gw60_sm);
      }
      else // already in pos
      {
        gw60_new_ref_pos = false;
        gw60_sm = GW60_SM_WAIT_4_REFPOS;
        XsnsGw60EverySecond();
        AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "already in pos"));
        LogSm(old_gw60_sm, gw60_sm);
      }
    }

    else // pos not possible - homing missed
    {
      gw60_new_ref_pos = false;
      gw60_sm = GW60_SM_WAIT_4_REFPOS;
      XsnsGw60EverySecond();
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "homing missed"));
      LogSm(old_gw60_sm, gw60_sm);
    }

    break;

  // 4 GW60_SM_CHECK_UP_DOWN_MOVE ==========================================================================
  case GW60_SM_CHECK_UP_DOWN_MOVE:
    if (digitalRead(Pin(GPIO_GW60_MOTOR_1)) && digitalRead(Pin(GPIO_GW60_MOTOR_2)))
    {

      digitalWrite(Pin(GPIO_GW60_UP), true);
      digitalWrite(Pin(GPIO_GW60_DOWN), true);

      if (gw60_ref_pos_sm == GW60_MIN_POS)
      {
        gw60_up_after_init = false;

        if (!gw60_homing_up)
        {
          gw60_scounter = 0;
          gw60_homing_up = true;
          AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "homing up done"));
        }

        if (gw60_auto_homing)
        {
          gw60_ref_pos = GW60_MAX_POS;
          AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "auto homing REFPOS=%d"), gw60_ref_pos);
        }
        else
        {
          if (gw60_homing)
            gw60_scounter = (gw60_scounter <= GW60_INACURRACY_SCOUNTER) ? 0 : gw60_scounter; // if homing ok adjust motion inaccuracy
          gw60_new_ref_pos = false;
        }
      }

      else // if (gw60_ref_pos_sm == GW60_MAX_POS)
      {
        if (gw60_homing_up)
        {
          if (!gw60_homing)
          {
            Settings.gw60_max_scounter = gw60_max_scounter = (gw60_scounter > 0) ? gw60_scounter : GW60_MAX_SCOUNTER; // avoid div by 0
            gw60_homing = true;
            gw60_auto_homing = false;

            AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "homing ok msc=%d"), gw60_max_scounter);
          }
          else
          {
            gw60_scounter = (gw60_scounter >= (gw60_max_scounter - GW60_INACURRACY_SCOUNTER)) ? gw60_max_scounter : gw60_scounter; // if homing ok adjust motion inaccuracy
          }
        }
        gw60_new_ref_pos = false;
      }

      gw60_sm = GW60_SM_WAIT_4_REFPOS;
      XsnsGw60EverySecond();
      LogSm(old_gw60_sm, gw60_sm);
    }
    break;

  // 5 GW60_SM_CHECK_POS_MOVE_UP ========================================================================
  case GW60_SM_CHECK_POS_MOVE_UP:
    if (gw60_scounter <= gw60_ref_scounter)
    {
      digitalWrite(Pin(GPIO_GW60_UP), false); // force stop
      digitalWrite(Pin(GPIO_GW60_DOWN), false);

      gw60_sm = GW60_SM_CHECK_POS_STOP;
      XsnsGw60EverySecond();
      LogSm(old_gw60_sm, gw60_sm);
    }
    else if (digitalRead(Pin(GPIO_GW60_MOTOR_1)) && digitalRead(Pin(GPIO_GW60_MOTOR_2)))
    {
      digitalWrite(Pin(GPIO_GW60_UP), true);
      digitalWrite(Pin(GPIO_GW60_DOWN), true);
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "Error REFPOS=%d not reached"), gw60_ref_pos_sm);

      gw60_new_ref_pos = false;
      gw60_sm = GW60_SM_WAIT_4_REFPOS;
      XsnsGw60EverySecond();
      LogSm(old_gw60_sm, gw60_sm);
    }
    break;

  // 6 GW60_SM_CHECK_POS_MOVE_DOWN ========================================================================
  case GW60_SM_CHECK_POS_MOVE_DOWN:
    if (gw60_scounter >= gw60_ref_scounter)
    {
      digitalWrite(Pin(GPIO_GW60_UP), false); // force stop
      digitalWrite(Pin(GPIO_GW60_DOWN), false);

      gw60_sm = GW60_SM_CHECK_POS_STOP;
      XsnsGw60EverySecond();
      LogSm(old_gw60_sm, gw60_sm);
    }
    else if (digitalRead(Pin(GPIO_GW60_MOTOR_1)) && digitalRead(Pin(GPIO_GW60_MOTOR_2)))
    {
      digitalWrite(Pin(GPIO_GW60_UP), true);
      digitalWrite(Pin(GPIO_GW60_DOWN), true);
      AddLog_P2(LOG_LEVEL_DEBUG, PSTR(D_LOG_GW60 D_XSNS_GW60_HANDLER "Error REFPOS=%d not reached"), gw60_ref_pos_sm);

      gw60_new_ref_pos = false;
      gw60_sm = GW60_SM_WAIT_4_REFPOS;
      XsnsGw60EverySecond();
      LogSm(old_gw60_sm, gw60_sm);
    }
    break;

  // 7 GW60_SM_CHECK_POS_STOP =================================================
  case GW60_SM_CHECK_POS_STOP:
    if (digitalRead(Pin(GPIO_GW60_MOTOR_1)) && digitalRead(Pin(GPIO_GW60_MOTOR_2)))
    {
      digitalWrite(Pin(GPIO_GW60_UP), true);
      digitalWrite(Pin(GPIO_GW60_DOWN), true);

      gw60_new_ref_pos = false;
      gw60_sm = GW60_SM_WAIT_4_REFPOS;
      XsnsGw60EverySecond();
      LogSm(old_gw60_sm, gw60_sm);
    }

  default:
    break;
  }
}

/*********************************************************************************************\
 * Xsns91 GW60 Main Interface
\*********************************************************************************************/
bool Xsns91(uint8_t function)
{
  //AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "Xsns91() fun=%d"), function);
  bool result = false;

  switch (function)
  {
  case FUNC_INIT:
    result = XsnsGw60Init();
    break;

  case FUNC_EVERY_SECOND:
    if (gw60_ok)
      result = XsnsGw60EverySecond();
    break;

  case FUNC_JSON_APPEND:
    if (gw60_ok)
      result = XsnsGw60Show(true);
    break;

#ifdef USE_WEBSERVER
  case FUNC_WEB_SENSOR:
    if (gw60_ok)
      result = XsnsGw60Show(false);
    break;
#endif // USE_WEBSERVER

  case FUNC_EVERY_250_MSECOND:
    if (gw60_ok)
      result = XsnsGW60Handler();
    break;

  case FUNC_COMMAND:
    if (gw60_ok)
      result = XsnsGw60Command();
    break;

  default:
    //AddLog_P2(LOG_LEVEL_DEBUG_MORE, PSTR(D_LOG_GW60 "Xsns91() default: fun=%d"), function);
    break;
  }

  return result;
}

/*********************************************************************************************\
 * Xsns91 Internal
\*********************************************************************************************/

/*********************************************************************************************\
 Gw60Interrupt
 * Process interrupts from hall sensor
 * gw60_scounter is incremented/decremented depending on the direction of the blind movement
  
  Remark: hall sensor will generate four interrupts per revolution of the input wheel
          ISR has to be located in IRAM!
\*********************************************************************************************/
void XsnsGw60Interrupt(void)
{
  bool motor_r = digitalRead(Pin(GPIO_GW60_MOTOR_1));
  bool motor_l = digitalRead(Pin(GPIO_GW60_MOTOR_2));
  if (motor_r && !motor_l)
  {
    gw60_scounter = (++gw60_scounter > GW60_MAX_SCOUNTER) ? GW60_MAX_SCOUNTER : gw60_scounter;
  }
  else if (motor_l && !motor_r)
  {
    gw60_scounter = (--gw60_scounter < 0) ? 0 : gw60_scounter;
  }
}

#endif // USE_GW60