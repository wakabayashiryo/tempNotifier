#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <string>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "Ambient.h"

#define MinutesIs(t) (60000*t)

//device configrations
//device:esp8266_generic_
//CpuFrequency_160MHz
//VTable_flash
//ResetMethod_ck
//CrystalFreq_26MHz
//FlashFreq_80MHz
//FlashMode_qio
//FlashSize_4M1M
//led_2
//LwIPVariant_v2mss536
//Debug_Disabled
//DebugLevel_None
//FlashErase_none
//UploadSpeed_115200bps
//Must Set Crystal Frequency of board configration to 26MHz

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

void WiFiconnect(void);
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
 
  dht.begin();
  
  if(!bmp.begin())
  {
    digitalWrite(STAT_ERROR ,LOW);
    while(1)delay(1);
  }
  
  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  SSIDs.addAP("4CE676F701EA",  "116mt8vyhx91w");
  SSIDs.addAP("4CE676F701EA-1","116mt8vyhx91w");
  SSIDs.addAP("aterm-912afc-g","39f9398943819");

  WiFiconnect();

  digitalWrite(STAT_WIFI ,LOW);

  ambient.begin(AMBIENT_ID,AMBIENT_KEY,&client);

}

void loop()
{  
  digitalWrite(STAT_ACT ,LOW);

  if(WiFi.status()!=WL_CONNECTED)
  {
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
  
  dat["value1"] = bmp.readTemperature();
  dat["value2"] = bmp.readPressure();
  dat["value3"] = floor(0.81*temp+0.01*humid*(0.99*temp-14.3)+46.3);   //calculate heat-index
  
  // Use WiFiClient class to create TCP connections
  if (!client.connect(IFTTT_HOST_NAME, PORT_NUMBER)) 
  {
    return;
  }
  
  // Create HTML Packets sent to IFTTT
  String Packets;
  Packets  = "POST https://maker.ifttt.com/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + String(IFTTT_KEY) + "/ "+"HTTP/1.1\r\n";
  Packets += "Host:maker.ifttt.com\r\n";
  Packets += "Content-Length:" + String(dat.measureLength()) + "\r\n";
  Packets += "Content-Type: application/json\r\n\r\n";
  dat.printTo(Packets);
  Packets += "\r\n";
  
//  Serial.println(Packets);
  
  // This will send the request to the server
  client.print(Packets);
  
  int timeout = millis() + 5000;
  while (client.available() == 0) 
  {
    if (timeout - millis() < 0) 
    {
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) 
  {
      client.readStringUntil('\r');
  }

  ambient.send();
  
  digitalWrite(STAT_ACT ,HIGH);
  
  delay(MinutesIs(1));
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
     
  wifi_set_sleep_type(MODEM_SLEEP_T);
  Serial.println("\nConnected to "+String(WiFi.SSID())+"\nIP address : "+WiFi.localIP().toString());
}

