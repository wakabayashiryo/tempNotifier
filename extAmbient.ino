#include <time.h>
#include "Ambient.h"

#define _DEBUG            0

//definitation of Ambient
#define AMBIENT_ID        6469
#define AMBIENT_KEY       "62ebf86863cc7a62"  
Ambient ambient;

#define JST               3600*9
time_t t;
struct tm *tm;

//definitation of buffer
#define _BUFF_MAXSIZE     50000
#define _ALLOW_SPACE      500
#define _REALLOC_SIZE     1000

static char*    payload;
static uint32_t payload_size;
static uint32_t buffer_size;


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

bool extAmbient_Send(void)
{
  if(ambient.send() == false)
  {
#if _DEBUG
    Serial.println("[error]:Filed to sent a data");
#endif
    return false;
  }
  return true;
}


static int32_t Check_Size(uint32_t size)
{
  payload_size += size;
  
  if((buffer_size - payload_size)  < _ALLOW_SPACE)
  {
    if((buffer_size +_REALLOC_SIZE) > _BUFF_MAXSIZE)
    {
#if _DEBUG
    Serial.println("[error]:Buffer size exceeded maximum");
#endif
      return -1;
    }

    char *res = (char *)realloc(payload,sizeof(char*) *_REALLOC_SIZE);
    if(res == NULL)
    {
#if _DEBUG
    Serial.println("[error]:Failed to add new buffer");
#endif
      return -2;
    }
    buffer_size += _REALLOC_SIZE;
  }
#if _DEBUG
    Serial.print("[data]:wrote size:"+ String(size) +"bytes ");
    Serial.print("buffer_size:"+ String(buffer_size) +"bytes ");
    Serial.println("payload_size:"+ String(payload_size) +"bytes");
#endif
  
  return 0;
}

int32_t extAmbient_Create_Buffer(uint32_t buffersize)
{
  int32_t result = 0;
  
  if(buffersize >= _BUFF_MAXSIZE)
  {
#if _DEBUG
    Serial.println("[error]:A buffer value exceeding the maximum size was entered");
#endif
    buffersize = _BUFF_MAXSIZE;
    result = -1;
  }
     
  payload = (char*)malloc(sizeof(char*) * buffersize);
  if(payload == NULL)
  {
#if _DEBUG
    Serial.println("[error]:Failed to add buffer");
#endif
    result = -2;
  }
  buffer_size  = buffersize;
  payload_size = 0;
  memset(payload,NULL,buffersize);
  
  Check_Size(sprintf(&payload[payload_size],"{\"writeKey\":\"%s\",\"data\":[",AMBIENT_KEY));

  return result;
}

/*json format for sending ambient by bulk
{
  "writeKey":"ライトキー",
  "data":[
    {"created":"YYYY-MM-DD HH:MM:SS.SSS","d1":値,"d2":値,"d3":値,"d4":値}
  ]
}
*/

static uint16_t time2format(struct tm *tms)
{ 
  uint16_t size;
  
  size = sprintf(&payload[payload_size],"\"%u-%u-%u %u:%u:%u.000\",",\
  tms->tm_year+1900,                                 \
  tms->tm_mon+1,                                     \
  tms->tm_mday,                                      \
  tms->tm_hour,                                      \
  tms->tm_min,                                       \
  tms->tm_sec                                        \
  );
  
  return size;
}

static uint16_t data2format(float d1,float d2,float d3,float d4)
{
  uint16_t size;
  
  size = sprintf(&payload[payload_size],"\"d1\":%.2f,\"d2\":%.2f,\"d3\":%.2f,\"d4\":%.2f",d1,d2,d3,d4);
  
  return size;
}

void extAmbient_Store(float d1,float d2,float d3,float d4)
{
  t  = time(NULL);
  tm = localtime(&t);
  
  Check_Size(sprintf(&payload[payload_size],"{\"created\":"));
  Check_Size(time2format(tm));
  Check_Size(data2format(d1,d2,d3,d4));
  Check_Size(sprintf(&payload[payload_size],"},"));
}

int32_t extAmbient_BulkSend(void)
{
  int32_t sentNum;

  Check_Size(sprintf(&payload[payload_size-1],"]}\r\n"));
  
  sentNum = ambient.bulk_send(payload);
  
  free(payload);
  
#if _DEBUG
    Serial.println("[data]:\n" + String(payload));
    Serial.print("[data]:payload_size:"+ String(strlen(payload)) +"bytes ");
    Serial.println("sent_size:"+ String(sentNum) +"bytes");
#endif
  
  if(sentNum != strlen(payload))
  {
#if _DEBUG
    Serial.println("[error]:Filed to sent bulk data");
#endif
    return -1;
  }
  
  return 0;
}
