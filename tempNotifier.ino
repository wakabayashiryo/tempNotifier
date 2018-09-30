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

#define STAT_ACT          12
#define STAT_ERROR        13
#define STAT_WIFI         16

#define DHT_PIN           14
#define DHT_TYPE          DHT11
float temp,humid,press,heatindex;
DHT dht(DHT_PIN,DHT_TYPE);

Adafruit_BMP280 bmp;

//definitation of Ambient
#define AMBIENT_ID        6469
#define AMBIENT_KEY       "62ebf86863cc7a62"  

Ambient ambient;

WiFiClient client;
ESP8266WiFiMulti SSIDs;

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
  SSIDs.addAP("4CE676F701EA",  "");
  SSIDs.addAP("4CE676F701EA-1","");
  SSIDs.addAP("aterm-912afc-g","");

  WiFiconnect();

  
  dht.begin();
  
  if(!bmp.begin())
  {
    digitalWrite(STAT_ERROR ,LOW);
    Send2LINE("error:","Did not initialize BMP280!");
  }
  
  ambient.begin(AMBIENT_ID,AMBIENT_KEY,&client);
  
  Send2LINE("info","Start Enviroment Monitor!");
}

void loop()
{  
  if(WiFi.status()!=WL_CONNECTED)
  {    
    digitalWrite(STAT_ERROR ,LOW);
    
    WiFi.disconnect();
    delay(100);
    WiFiconnect();
     
    digitalWrite(STAT_ERROR ,HIGH);
  }
  
  temp  = dht.readTemperature();
  humid = dht.readHumidity();
  press = bmp.readPressure();
  heatindex = floor(0.81*temp+0.01*humid*(0.99*temp-14.3)+46.3);
  
  if(heatindex<=60&&80<=heatindex)
    Send2LINE("warinig","It is an unpleasant environment now. ["+String(heatindex)+"]");
  
  ambient.set(1,temp);
  ambient.set(2,humid);
  ambient.set(3,press/100);
  ambient.set(4,heatindex);

  digitalWrite(STAT_ACT ,LOW);
  
  if(ambient.send() == false)
  {
    Send2LINE("error","Did not send to ambient.");
  }

  digitalWrite(STAT_ACT ,HIGH);
  
  delay(WakePeriod*60000);
}



