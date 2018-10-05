#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti SSIDs;

#define _SCAN_TIMEOUT   30000//unit:mili second

void SSIDregister(void)
{
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  SSIDs.addAP("4CE676F701EA",  "");
  SSIDs.addAP("4CE676F701EA-1","");
  SSIDs.addAP("aterm-912afc-g","");
}

bool WiFiconnect(void)
{
  int32_t timeout = (int32_t)millis() + _SCAN_TIMEOUT;

  while(SSIDs.run() != WL_CONNECTED) 
  {
    if (timeout - (int32_t)millis() < 0) 
    {
      Serial.println("[error]:Access point connection timed out.");
      return false;
    }
    delay(500);
    digitalWrite(STAT_WIFI ,HIGH);
    delay(500);
    digitalWrite(STAT_WIFI ,LOW);
  }
       
  wifi_set_sleep_type(MODEM_SLEEP_T);
  
  Send2LINE("WiFi info","SSID:"+String(WiFi.SSID())+"  IPaddress:"+WiFi.localIP().toString());
  
  return true;
}
