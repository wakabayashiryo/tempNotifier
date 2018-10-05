#include <time.h>
#include "Ambient.h"

//definitation of Ambient
#define AMBIENT_ID        6469
#define AMBIENT_KEY       "62ebf86863cc7a62"  
Ambient ambient;

#define JST               3600*9
time_t t;
struct tm *tm;

#define _MAX_PAYLOAD      50000
#define _ALLOW_SPACE      500
#define _REALLOC_SIZE     1000
static char     *payload;
static uint32_t payload_size;

static void Check_Size(uint32_t size)
{
  uint32_t buffer_size = sizeof(payload)/sizeof(char*);
  payload_size += size;
  
  if((buffer_size - payload_size) < _ALLOW_SPACE)
  {
    if(buffer_size+_REALLOC_SIZE>_MAX_PAYLOAD)
    {
      Send2LINE("error","RAM is not large enough space");
      return ;
    }

    char *res = (char *)realloc(payload,sizeof(char*)*_REALLOC_SIZE);
    if(res == NULL)
    {
        Send2LINE("error","failed realloc function.");
    }
  }
}

void extAmbient_Create_Buffer(uint32_t buffersize)
{
  if(buffersize >= _MAX_PAYLOAD)
  {
    buffersize = _MAX_PAYLOAD;
    Send2LINE("error","The value you specifyed is over max size.");
  }
     
  payload = (char *)malloc(sizeof(char*)*buffersize);
  if(payload == NULL)
  {
      Send2LINE("error","Did not secure buffer for back up.");
  }
  
  payload_size = 0;
  memset(payload,NULL,strlen(payload));
  
  Check_Size(sprintf(payload,"{ \"writeKey\" : \"%s\",",AMBIENT_KEY));
  Check_Size(sprintf(payload," \"data\" : [ "));
}

void extAmbient_Init(WiFiClient *clt)
{
  ambient.begin(AMBIENT_ID,AMBIENT_KEY,clt);
  
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  extAmbient_Create_Buffer(5000);
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

static uint16_t time2json(struct tm *tms)
{ 
   return sprintf(payload,"\"%u-%u-%u %u:%u:&u.000\",",\
   tms->tm_year+1900,                                 \
   tms->tm_mon+1,                                     \
   tms->tm_mday,                                      \
   tms->tm_hour,                                      \
   tms->tm_min,                                       \
   tms->tm_sec                                        \
   );
}

static uint16_t data2json(float d1,float d2,float d3,float d4)
{
 return sprintf(payload,"\"d1\":\"%f\",\"d2\":\"%f\",\"d3\":\"%f\",\"d4\":\"%f\",",\
                                  d1,           d2,           d3,           d4);
}

void extAmbient_Store(float d1,float d2,float d3,float d4)
{
  t  = time(NULL);
  tm = localtime(&t);
  
  Check_Size(sprintf(payload,"{ \"created\" : "));
  Check_Size(time2json(tm));
  Check_Size(data2json(d1,d2,d3,d4));
  Check_Size(sprintf(payload,"}."));
}

void extAmbient_BulkSend(void)
{
  Check_Size(sprintf(payload,"]}\r\n"));
  
  uint8_t sentNum = ambient.bulk_send(payload);

  Serial.println("Contents of payload\n"+String(payload));
  Serial.println("sent size["+String(sentNum)+"]Bytes");  

  free(payload);
}

