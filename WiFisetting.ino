
void WiFiconnect(void)
{
  while(SSIDs.run() != WL_CONNECTED) 
  {
    delay(500);
    digitalWrite(STAT_WIFI ,HIGH);
    delay(500);
    digitalWrite(STAT_WIFI ,LOW);
  }
     
  wifi_set_sleep_type(MODEM_SLEEP_T);
  
  Send2LINE("WiFi info","SSID:"+String(WiFi.SSID())+"  IPaddress:"+WiFi.localIP().toString());
}
