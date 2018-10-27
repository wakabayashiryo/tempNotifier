#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti SSIDs;

#define _SCAN_TIMEOUT   30000//unit:mili second

void SSIDregister(void)
{
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  SSIDs.addAP("4CE676F701EA",  "");
  SSIDs.addAP("4CE676F701EA-1","");
  SSIDs.addAP("F660A-T2G6-G",  "");
}

bool WiFiconnect(void)
{
  int32_t timeout = (int32_t)millis() + _SCAN_TIMEOUT;

  while(SSIDs.run() != WL_CONNECTED) 
  {
    if (timeout - (int32_t)millis() < 0) 
    {
      return false;
    }
    delay(500);
    digitalWrite(_STAT_WIFI ,HIGH);
    delay(500);
    digitalWrite(_STAT_WIFI ,LOW);
  }
       
  wifi_set_sleep_type(MODEM_SLEEP_T);
    
  return true;
}
