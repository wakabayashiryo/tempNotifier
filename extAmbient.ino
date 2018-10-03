#include <time.h>
#include<FS.h>
#include "Ambient.h"

//definitation of Ambient
#define AMBIENT_ID        6469
#define AMBIENT_KEY       "62ebf86863cc7a62"  

Ambient ambient;
File fp;

#define JST     3600*9
time_t t;
struct tm *tm;

String bulkdata ;

void extAmbient_Generate_File(void)
{
  fp = SPIFFS.open("backup.txt","w");
  if(!fp)
  {
    Send2LINE("error","Can not open back-up file.");
  }
  fp.print("{ \"writeKey\" : \"" + String(AMBIENT_KEY) + "\",");
  fp.print(" \"data\" : [ ");
}

void extAmbient_Init(WiFiClient *clt)
{
  ambient.begin(AMBIENT_ID,AMBIENT_KEY,clt);
  
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  SPIFFS.begin();
  SPIFFS.format();
  extAmbient_Generate_File();
}

void extAmbient_Set(float d1,float d2,float d3,float d4)
{
  ambient.set(1,d1);
  ambient.set(2,d2);
  ambient.set(3,d3);
  ambient.set(4,d4);
}

void extAmbient_Send(void)
{
  if(ambient.send() == false)
  {
    Send2LINE("error","Did not send to ambient.");
  }
}

/*json format for sending ambient by bulk
{
  “writeKey” :  “ライトキー”,
  “data” : 
  [
      {“created” : “YYYY-MM-DD HH:mm:ss.sss”, “d1” :  “値”, “d2” :  “値”, ...},
  ]
}          
*/

static String time2json(struct tm *tms)
{ 
  String json;
  
  json  = " \""+ String(tms->tm_year+1900) + "-";
  json +=        String(tms->tm_mon+1    ) + "-";
  json +=        String(tms->tm_mday     ) + " ";
  json +=        String(tms->tm_hour     ) + ":";
  json +=        String(tms->tm_min      ) + ":";
  json +=        String(tms->tm_sec      ) + ".";
  json +=        "000";
  json += " \",";

  return json;
}

static String data2json(float d1,float d2,float d3,float d4)
{
  String json;
  
  json  = " \"d1\" : \"" + String(d1) + "\",";
  json += " \"d2\" : \"" + String(d2) + "\",";
  json += " \"d3\" : \"" + String(d3) + "\",";
  json += " \"d4\" : \"" + String(d4) + "\" ";

  return json;
}

void extAmbient_Store(float d1,float d2,float d3,float d4)
{
  t  = time(NULL);
  tm = localtime(&t);
  
  fp.print("{ \"created\" : " + time2json(tm) + data2json(d1,d2,d3,d4) + "},");
}

void extAmbient_BulkSend(void)
{
  fp.print("] }");
  
  fp.close();

  fp = SPIFFS.open("backup.txt","r");
  if(!fp)
  {
    Serial.print("can not open backup.txt");
  }

  bulkdata = fp.readStringUntil('\n');

  Serial.println((char *)bulkdata.c_str());
 
  uint8_t sentNum = ambient.bulk_send((char *)bulkdata.c_str());

  Serial.println(sentNum);
  fp.close();
  
  SPIFFS.remove("backup.txt");
  
}

