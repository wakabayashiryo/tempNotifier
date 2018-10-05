#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <string>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define WakePeriod        10//unit:minute

#define STAT_ACT          12
#define STAT_ERROR        13
#define STAT_WIFI         16

#define DHT_PIN           14
#define DHT_TYPE          DHT11
float temp,humid,press,heatindex;
DHT dht(DHT_PIN,DHT_TYPE);

Adafruit_BMP280 bmp;

WiFiClient client;

bool connectionIs = false;

void setup(void)
{
  pinMode(STAT_WIFI ,OUTPUT);
  pinMode(STAT_ERROR,OUTPUT);
  pinMode(STAT_ACT  ,OUTPUT);

  digitalWrite(STAT_WIFI ,HIGH);
  digitalWrite(STAT_ERROR,HIGH);
  digitalWrite(STAT_ACT  ,HIGH);
  
  Serial.begin(115200);

  SSIDregister();
  if(WiFiconnect()==true)
    connectionIs = true;
  else
    connectionIs = false;

  dht.begin();
  
  if(!bmp.begin())
  {
    digitalWrite(STAT_ERROR ,LOW);
    Send2LINE("error:","Did not initialize BMP280!");
  }

  extAmbient_Init(&client);
  
  Send2LINE("info","Start Enviroment Monitor!");
}

void loop(void)
{  
  temp  = dht.readTemperature();
  humid = dht.readHumidity();
  press = bmp.readPressure();
  heatindex = floor(0.81*temp+0.01*humid*(0.99*temp-14.3)+46.3);
  
  if(heatindex<=60&&80<=heatindex)
    Send2LINE("warinig","It is an unpleasant environment now. ["+String(heatindex)+"]");

  if(WiFi.status()!=WL_CONNECTED)
  {    
    digitalWrite(STAT_ERROR ,LOW);
    
    WiFi.disconnect();
    delay(100);
    if(WiFiconnect()==true)
    {
      connectionIs = true;
      extAmbient_BulkSend();
      digitalWrite(STAT_ERROR ,HIGH);
    }
    else if(connectionIs==true)
    {
      connectionIs = false;
      extAmbient_Generate_File();
    }
  }

  if(connectionIs==true)
  {
    digitalWrite(STAT_WIFI ,LOW);
    
    extAmbient_Set(temp,humid,press/100,heatindex); 
        
    digitalWrite(STAT_ACT ,LOW);
    extAmbient_Send();
    digitalWrite(STAT_ACT ,HIGH);
  }
  else
  {
    digitalWrite(STAT_WIFI ,HIGH);
    
    digitalWrite(STAT_ACT ,LOW);
    extAmbient_Store(temp,humid,press/100,heatindex);
    digitalWrite(STAT_ACT ,HIGH);
  }      
    
  delay(WakePeriod*60000);
}



