#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <string>
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

// host name of IFTTT
#define IFTTT_HOST_NAME   "maker.ifttt.com"

// event name of IFTTT
#define IFTTT_EVENT_NAME  "temp_notify"

// secret key of IFTTT
#define IFTTT_KEY         "cdjsD1Qw-5TOT4G3t53Zv_"

// port number
#define PORT_NUMBER       80

#define STAT_WIFI         12
#define STAT_ERROR        13
#define STAT_ACT          14

#define DHT_PIN           4
#define DHT_TYPE          DHT11
DHT dht(DHT_PIN,DHT_TYPE);

void connectWiFi(void);
WiFiClient client;
ESP8266WiFiMulti SSIDs;
 
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

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  SSIDs.addAP("4CE676F701EA", "116mt8vyhx91w");
  SSIDs.addAP("", "");
  SSIDs.addAP("", "");

  connectWiFi();

  digitalWrite(STAT_WIFI ,LOW);
}

void loop()
{  
  digitalWrite(STAT_ACT ,LOW);

  if(WiFi.status()!=WL_CONNECTED)
  {
    digitalWrite(STAT_ERROR ,LOW);
    
    WiFi.disconnect();
    delay(100);
    connectWiFi();
 
    digitalWrite(STAT_ERROR ,HIGH);
   }
  
  dat["value1"] = dht.readTemperature();
  dat["value2"] = dht.readHumidity();
  dat["value3"] = floor(dht.computeHeatIndex(dht.convertCtoF(dat["value1"]), dat["value2"]));
  
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
  Packets  = "POST http://maker.ifttt.com/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + String(IFTTT_KEY) + "/ HTTP/1.1\r\n";
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
  
  digitalWrite(STAT_ACT ,HIGH);
  
  delay(MinutesIs(15));
}

void connectWiFi(void)
{
  while(SSIDs.run() != WL_CONNECTED) 
  {
    delay(500);
    digitalWrite(STAT_WIFI ,HIGH);
    delay(500);
    digitalWrite(STAT_WIFI ,LOW);
  }
     
  wifi_set_sleep_type(MODEM_SLEEP_T);
  Serial.print("\nConnected to "+String(WiFi.SSID())+"\nIP address:\t"+String(WiFi.localIP()));
}

