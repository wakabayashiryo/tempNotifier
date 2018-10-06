#include <ArduinoJson.h>

#define _DEBUG            0

#define _CLIENT_TIMEOUT   5000

//definitation of IFTTT
#define IFTTT_HOST_NAME   "maker.ifttt.com"
#define IFTTT_EVENT_NAME  "temp_notify"
#define IFTTT_KEY         "cdjsD1Qw-5TOT4G3t53Zv_"
#define PORT_NUMBER       80

StaticJsonBuffer<200> jsonbuff;
JsonObject& dat = jsonbuff.createObject();

int32_t Send2LINE(String category,String message)
{
   
  Serial.println("[" + category + "]:" + message);
    
  dat["value1"] = category;
  dat["value2"] = message;
  
  // Use WiFiClient class to create TCP connections
  if (!client.connect(IFTTT_HOST_NAME, PORT_NUMBER)) 
  {
#if _DEBUG
    Serial.println("[error]:Did not connect with IFTTT server.");
#endif
    return -1;
  }
  
  // Create HTML Packets sent to IFTTT
  String Packets;
  Packets  = "POST https://maker.ifttt.com/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + String(IFTTT_KEY) + "/ "+"HTTP/1.1\r\n";
  Packets += "Host:maker.ifttt.com\r\n";
  Packets += "Content-Length:" + String(dat.measureLength()) + "\r\n";
  Packets += "Content-Type: application/json\r\n\r\n";
  dat.printTo(Packets);
  Packets += "\r\n";
#if _DEBUG
  Serial.println(Packets);
#endif
  
  // This will send the request to the server
  client.print(Packets);
    
  int32_t timeout = millis() + _CLIENT_TIMEOUT;
  while (client.available() == 0) 
  {
    if (timeout - (int32_t)millis() < 0) 
    {
      client.stop();
#if _DEBUG 
      Serial.println("[error]:Recieving response timed out from server.");
#endif
      return -2;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) 
  {
      //Flush message received from server
#if _DEBUG
      Serial.print(client.readStringUntil('\r'));
#else
      client.readStringUntil('\r');
#endif
  }
  
  return 0;
}
