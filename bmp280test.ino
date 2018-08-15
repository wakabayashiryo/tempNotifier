#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <SPI.h>
#include <string>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Wi-Fi SSID
#define WLAN_SSID         "4CE676F701EA"
// Wi-Fi パスワード
#define WLAN_PASS         "116mt8vyhx91w"

// IFTTTのホスト名
#define IFTTT_HOST_NAME  "maker.ifttt.com"

// IFTTTのイベント名
#define IFTTT_EVENT_NAME  "temp_notify"

// IFTTTのシークレットキー
#define IFTTT_KEY         "cdjsD1Qw-5TOT4G3t53Zv_"

// ポート番号
#define PORT_NUMBER       80

//Adafruit_BMP280 bme;

void setup() 
{
  Serial.begin(115200);
  Serial.println(F("Start notify system using IFTTT\n"));

//	Wire.begin(4,5);
//  if (!bme.begin()) 
//  {  
//    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
//    while (1);
//  }
  // We start by connecting to a WiFi network

  Serial.print("\n\nConnecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  StaticJsonBuffer<200> jsonbuff;
  JsonObject& dat = jsonbuff.createObject();

//  dat["value1"] = bme.readPressure();
//  dat["value2"] = bme.readTemperature();
//  dat["value3"] = bme.readAltitude(1013.25);
    
  Serial.print("connecting to ");
  Serial.println(IFTTT_HOST_NAME);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;  
  if (!client.connect(IFTTT_HOST_NAME, PORT_NUMBER)) {
    Serial.println("connection failed");
    return;
  }


// IFTTTへ送信するデータ
  
  String value_json;
  dat.printTo(value_json);
  value_json += "\r\n";
  
  String Packets;
	Packets = "POST http://maker.ifttt.com/trigger/";
	Packets += IFTTT_EVENT_NAME;
	Packets += "/with/key/";
	Packets += IFTTT_KEY;
	Packets += "/ HTTP/1.1\r\nHost:maker.ifttt.com\r\nContent-Length:";
	Packets += String(value_json.length());
  Packets += "\r\nContent-Type: application/json\r\n\r\n";
  Packets += value_json;
  Packets += "\r\n";
  
///  sprintf(sData,"POST http://maker.ifttt.com/trigger/%s/with/key/%s/ HTTP/1.1\r\nHost:maker.ifttt.com\r\nContent-Length:%d\r\nContent-Type: application/json\r\n\r\n%s\r\n",IFTTT_EVENT_NAME,IFTTT_KEY,contentlength,value.c_str());
  Serial.print(Packets);
  // This will send the request to the server
  client.print(Packets);
  int timeout = millis() + 5000;
  while (client.available() == 0) {
    if (timeout - millis() < 0) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");

  delay(5000);
}
