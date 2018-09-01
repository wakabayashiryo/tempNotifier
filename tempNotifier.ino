#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <string>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#define MinutesIs(t) (60000*t)
//Must Set Crystal Frequency of board configration to 26MHz


// host name of IFTTT
#define IFTTT_HOST_NAME   "maker.ifttt.com"

// event name of IFTTT
#define IFTTT_EVENT_NAME  "temp_notify"

// secret key of IFTTT
#define IFTTT_KEY         "cdjsD1Qw-5TOT4G3t53Zv_"

// ポート番号
#define PORT_NUMBER       80

#define STAT_WIFI         12
#define STAT_ERROR        13
#define STAT_WAKE         14

void failed_sleep(char *errmsg);

Adafruit_BMP280 bme;

ESP8266WiFiMulti SSIDs;

WiFiClient client;
 
StaticJsonBuffer<200> jsonbuff;
JsonObject& dat = jsonbuff.createObject();

void setup()
{
  pinMode(STAT_WIFI ,OUTPUT);
  pinMode(STAT_ERROR,OUTPUT);
  pinMode(STAT_WAKE ,OUTPUT);

  digitalWrite(STAT_WIFI ,HIGH);
  digitalWrite(STAT_ERROR,HIGH);
  digitalWrite(STAT_WAKE ,HIGH);
  
  Serial.begin(115200);

  Wire.begin(4, 5);
  if(!bme.begin())
  {
    failed_sleep("failed to initialize BMP280");
  }

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  SSIDs.addAP("", "");
  SSIDs.addAP("", "");
  SSIDs.addAP("", "");

  while(SSIDs.run() != WL_CONNECTED) 
  {
    delay(250);
    digitalWrite(STAT_WIFI ,HIGH);
    delay(250);
    digitalWrite(STAT_WIFI ,LOW);
  }
   
  digitalWrite(STAT_WIFI ,LOW);
  
  wifi_set_sleep_type(MODEM_SLEEP_T);
  Serial.print("\nConnected to "+String(WiFi.SSID())+"\nIP address:\t"+String(WiFi.localIP()));
}

void loop()
{  
  if(WiFi.status()!=WL_CONNECTED)
  {
    digitalWrite(STAT_ERROR ,LOW);
    WiFi.disconnect();
    delay(100);
    while(SSIDs.run()!=WL_CONNECTED)
    {
       Serial.print(".");
       delay(250);
    }
    digitalWrite(STAT_ERROR ,LOW);
    wifi_set_sleep_type(MODEM_SLEEP_T);
    Serial.print("\nReconnected to "+String(WiFi.SSID())+"\nIP address:\t"+String(WiFi.localIP()));
  }

  digitalWrite(STAT_WAKE ,LOW);
  delay(250);
  digitalWrite(STAT_WAKE ,HIGH);

  dat["value1"] = floor(bme.readPressure() / 100);
  dat["value2"] = bme.readTemperature();
  dat["value3"] = floor(bme.readAltitude(1013.25));
  
  // Use WiFiClient class to create TCP connections
  if (!client.connect(IFTTT_HOST_NAME, PORT_NUMBER)) 
  {
    return;
  }
  
  // Create HTML Packets sent to IFTTT
  String value_json;
  dat.printTo(value_json);
  value_json += "\r\n";
  
  String Packets;
  Packets =  "POST http://maker.ifttt.com/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + String(IFTTT_KEY) + "/ HTTP/1.1\r\n";
  Packets += "Host:maker.ifttt.com\r\n";
  Packets += "Content-Length:" + String(value_json.length()) + "\r\n";
  Packets += "Content-Type: application/json\r\n\r\n";
  Packets += value_json + "\r\n";
  
  ///  Serial.print(Packets);
  
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
  
  delay(MinutesIs(15));
}
    
void failed_sleep(char *errmsg)
{
  Serial.println("\n"+String(errmsg));
  
  digitalWrite(STAT_ERROR,LOW);
  
  while(1)delay(1000);
}



