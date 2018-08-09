#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// 転送レート
#define SERIAL_SPEED  115200

// Wi-Fi SSID
#define WLAN_SSID         "aterm-912afc-g"
// Wi-Fi パスワード
#define WLAN_PASS         "39f9398943819"

// IFTTTのホスト名
#define IFTTT_HOST_NAME  "maker.ifttt.com"

// IFTTTのイベント名
#define IFTTT_EVENT_NAME  "temp_notify"

// IFTTTのシークレットキー
#define IFTTT_KEY         "cdjsD1Qw-5TOT4G3t53Zv_"

// ポート番号
#define PORT_NUMBER       80

void setup() {
  Serial.begin(SERIAL_SPEED);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
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

  dat["value1"] = "日本語";
  
  Serial.print("connecting to ");
  Serial.println(IFTTT_HOST_NAME);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;  
  if (!client.connect(IFTTT_HOST_NAME, PORT_NUMBER)) {
    Serial.println("connection failed");
    return;
  }

  // IFTTTへ送信するデータ

  char sData[200];
  String value;
  dat.printTo(value);
  value += "\r\n";
  int contentlength = value.length();
  
  sprintf(sData,"POST http://maker.ifttt.com/trigger/%s/with/key/%s/ HTTP/1.1\r\nHost:maker.ifttt.com\r\nContent-Length:%d\r\nContent-Type: application/json\r\n\r\n%s\r\n",IFTTT_EVENT_NAME,IFTTT_KEY,contentlength,value.c_str());
  Serial.print(sData);
  // This will send the request to the server
  client.print(sData);
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
