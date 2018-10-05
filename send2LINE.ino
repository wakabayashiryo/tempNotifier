
// definitation of IFTTT
#define IFTTT_HOST_NAME   "maker.ifttt.com"
#define IFTTT_EVENT_NAME  "temp_notify"
#define IFTTT_KEY         "cdjsD1Qw-5TOT4G3t53Zv_"
#define PORT_NUMBER       80

StaticJsonBuffer<200> jsonbuff;
JsonObject& dat = jsonbuff.createObject();

void Send2LINE(String category,String message)
{
  if(category == "error")
    digitalWrite(STAT_ERROR ,LOW);
    
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

  digitalWrite(STAT_ACT ,LOW);
  
  // This will send the request to the server
  client.print(Packets);
    
  int32_t timeout = millis() + 5000;
  while (client.available() == 0) 
  {
    if (timeout - (int32_t)millis() < 0) 
    {
      client.stop();
      
      digitalWrite(STAT_ERROR ,LOW);
      Serial.println("[error]:Recieving response timed out from server.");
      return ;
    }
  }
  digitalWrite(STAT_ACT ,HIGH);
  
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) 
  {
      //Flush message received from server
      //Serial.print(client.readStringUntil('\r'));
      client.readStringUntil('\r');
  }

}
