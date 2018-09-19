#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <string>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "Ambient.h"

#define WakePeriod         5

// definitation of IFTTT
#define IFTTT_HOST_NAME   "maker.ifttt.com"
#define IFTTT_EVENT_NAME  "temp_notify"
#define IFTTT_KEY         "cdjsD1Qw-5TOT4G3t53Zv_"
#define PORT_NUMBER       80

//definitation of Ambient
#define AMBIENT_ID        6469
#define AMBIENT_KEY       "62ebf86863cc7a62"  

#define STAT_ACT          12
#define STAT_ERROR        13
#define STAT_WIFI         16

#define DHT_PIN           14
#define DHT_TYPE          DHT11
DHT dht(DHT_PIN,DHT_TYPE);

Adafruit_BMP280 bmp;

WiFiClient client;
ESP8266WiFiMulti SSIDs;

Ambient ambient;
 
StaticJsonBuffer<200> jsonbuff;
JsonObject& dat = jsonbuff.createObject();

void setup()
{
  pinMode(STAT_WIFI ,OUTPUT);
  pinMode(STAT_ERROR,OUTPUT);
  pinMode(STAT_ACT  ,OUTPUT);

  digitalWrite(STAT_WIFI ,HIGH);
  digitalWrite(STAT_ERROR,HIGH);
  digitalWrite(STAT_ACT  ,HIGH);
  
  Serial.begin(115200);
 
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  SSIDs.addAP("4CE676F701EA",  "116mt8vyhx91w");
  SSIDs.addAP("4CE676F701EA-1","116mt8vyhx91w");
  SSIDs.addAP("aterm-912afc-g","39f9398943819");

  WiFiconnect();

  
  dht.begin();
  
  if(!bmp.begin())
  {
    digitalWrite(STAT_ERROR ,LOW);
    Send2LINE("error:","Did not initialize BMP280!");
  }
  
  ambient.begin(AMBIENT_ID,AMBIENT_KEY,&client);
  
  Send2LINE("info","Start to run!");
}

void loop()
{  
  if(WiFi.status()!=WL_CONNECTED)
  {
    Send2LINE("error","Disconnected WiFi.");
    
    digitalWrite(STAT_ERROR ,LOW);
    
    WiFi.disconnect();
    delay(100);
    WiFiconnect();
     
    digitalWrite(STAT_ERROR ,HIGH);
  }
  
  float temp  = dht.readTemperature();
  float humid = dht.readHumidity();
  float pres  = bmp.readPressure();

  ambient.set(1,temp);
  ambient.set(2,humid);
  ambient.set(3,pres/100);
  ambient.set(4,floor(0.81*temp+0.01*humid*(0.99*temp-14.3)+46.3));

  digitalWrite(STAT_ACT ,LOW);
  
  bool res = ambient.send();
  if(res==false)
  {
    Send2LINE("error","Did not send to ambient.");
  }
  
  digitalWrite(STAT_ACT ,HIGH);
  
  delay(WakePeriod*60000);
}

void WiFiconnect(void)
{
  while(SSIDs.run() != WL_CONNECTED) 
  {
    delay(500);
    digitalWrite(STAT_WIFI ,HIGH);
    delay(500);
    digitalWrite(STAT_WIFI ,LOW);
  }
  digitalWrite(STAT_WIFI ,LOW);
     
  wifi_set_sleep_type(MODEM_SLEEP_T);
  
  Send2LINE("WiFi info","SSID:"+String(WiFi.SSID())+"  IPaddress:"+WiFi.localIP().toString());
}

void Send2LINE(String category,String message)
{
  Serial.println("[" + category + "]:" + message);
  
  dat["value1"] = category;
  dat["value2"] = message;
  
  // Use WiFiClient class to create TCP connections
  if (!client.connect(IFTTT_HOST_NAME, PORT_NUMBER)) 
  {
    digitalWrite(STAT_ERROR ,LOW);
    Serial.println("[error]:Did not connect with IFTTT server.");
    return ;
  }
  
  // Create HTML Packets sent to IFTTT
  String Packets;
  Packets  = "POST https://maker.ifttt.com/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + String(IFTTT_KEY) + "/ "+"HTTP/1.1\r\n";
  Packets += "Host:maker.ifttt.com\r\n";
  Packets += "Content-Length:" + String(dat.measureLength()) + "\r\n";
  Packets += "Content-Type: application/json\r\n\r\n";
  dat.printTo(Packets);
  Packets += "\r\n";
  //Serial.println(Packets);
  
  // This will send the request to the server
  client.print(Packets);
  
  static int32_t timeout = millis() + 5000;
  while (client.available() == 0) 
  {
    if (timeout - millis() < 0) 
    {
      client.stop();
      
      digitalWrite(STAT_ERROR ,LOW);
      Serial.println("[error]:Time out recieved response from server.");
      return ;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) 
  {
      //Flush message received from server
      //Serial.print(client.readStringUntil('\r'));
      client.readStringUntil('\r');
  }
  
}
