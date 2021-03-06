#include <ESP8266WiFi.h>
#include <Wire.h>
#include <string>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define _WakePeriod       10//unit:minute

#define _STAT_ACT         12
#define _STAT_ERROR       13
#define _STAT_WIFI        16

#define _DHT_PIN          14
#define _DHT_TYPE         DHT11
float temp,humid,press,heatindex;
DHT dht(_DHT_PIN,_DHT_TYPE);

Adafruit_BMP280 bmp;

WiFiClient client;

bool connectionIs = false;

void setup(void)
{
  pinMode(_STAT_WIFI ,OUTPUT);
  pinMode(_STAT_ERROR,OUTPUT);
  pinMode(_STAT_ACT  ,OUTPUT);

  digitalWrite(_STAT_WIFI ,HIGH);
  digitalWrite(_STAT_ERROR,HIGH);
  digitalWrite(_STAT_ACT  ,HIGH);
  
  Serial.begin(115200);

  SSIDregister();
  if(WiFiconnect()==true)
    connectionIs = true;
  else
    connectionIs = false;

  dht.begin();
  
  if(!bmp.begin())
  {
    Send2LINE("error:","Did not initialize BMP280!");
  }

  extAmbient_Init(&client);
  
  Send2LINE("WiFi info","SSID:"+String(WiFi.SSID())+"  IPaddress:"+WiFi.localIP().toString());
  Send2LINE("info","Start Enviroment Monitor!");
}

void loop(void)
{    
  temp      = dht.readTemperature();
  humid     = dht.readHumidity();
  press     = bmp.readPressure()/100;
  heatindex = floor(0.81*temp+0.01*humid*(0.99*temp-14.3)+46.3);
  
  if( (heatindex<=60) || (80<=heatindex) )
    Send2LINE("warinig","It is an unpleasant environment now. ["+String(heatindex)+"]");

  if(WiFi.status()!=WL_CONNECTED)
  {    
    digitalWrite(_STAT_ERROR ,LOW);
    
    WiFi.disconnect();
    delay(100);
    if(WiFiconnect()==true)
    {
      connectionIs = true;
      extAmbient_BulkSend();
      digitalWrite(_STAT_ERROR ,HIGH);
      delay(_WakePeriod*60000);
      return ;
    }
    else if(connectionIs==true)
    {
      connectionIs = false;
      extAmbient_Create_Buffer(5000);
    }
  }

  if(connectionIs==true)
  {
    digitalWrite(_STAT_WIFI ,LOW);
    
    extAmbient_Set(temp,humid,press,heatindex); 
        
    digitalWrite(_STAT_ACT ,LOW);
    extAmbient_Send();
    digitalWrite(_STAT_ACT ,HIGH);
  }
  else
  {
    digitalWrite(_STAT_WIFI ,HIGH);
    
    digitalWrite(_STAT_ACT ,LOW);
    extAmbient_Store(temp,humid,press,heatindex);
    digitalWrite(_STAT_ACT ,HIGH);
  }      
  
  delay(_WakePeriod*60000);
}
