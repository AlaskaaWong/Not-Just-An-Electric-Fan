#include <avr/sleep.h>
#include <avr/power.h>
#include <IRremote.h>
#include <U8glib.h>

#define INTERVAL_SENSOR   5           
#define INTERVAL_NET      5         

#include <Wire.h>                                  
#include "./ESP8266.h"
#include "I2Cdev.h"                               
//温湿度   
#include <SHT2x.h>
//光照

#define  sensorPin_1  A0

#define SSID           "***"                   
#define PASSWORD       "***"

#define IDLE_TIMEOUT_MS  3000     
   
#define HOST_NAME   "***"
#define DEVICEID   "***"
#define PROJECTID "***"
#define HOST_PORT   (80)
String apiKey="bo5gyTwf65QKZSH63O2WR9cOF1Y=";
char buf[10];

#define INTERVAL_sensor 2000
unsigned long sensorlastTime = millis();


String mCottenData;
String jsonToSend;

//3,传感器值的设置 
float sensor_tem, sensor_hum;                    //传感器温度、湿度、光照   
char  sensor_tem_c[7], sensor_hum_c[7] ;    //换成char数组传输
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); /* RX:D3, TX:D2 */
ESP8266 wifi(mySerial);
//ESP8266 wifi(Serial1);                                      //定义一个ESP8266（wifi）的对象
unsigned long net_time1 = millis();                          //数据上传服务器时间
unsigned long sensor_time = millis();                        //传感器采样时间计时器

//int SensorData;                                   //用于存储传感器数据
String postString;                                //用于存储发送数据的字符串
//String jsonToSend;                                //用于存储发送的json格式参数

int RECV_PIN = 10;
IRrecv irrecv(RECV_PIN);
decode_results results;






 
long previousMillis = 0;        // 存储LED最后一次的更新
long interval = 5000;           // 闪烁的时间间隔（毫秒）
unsigned long currentMillis=0;


 
void setup(void)     //初始化函数  
{       
  //初始化串口波特率  
    Wire.begin();
    Serial.begin(115200); 
 
    irrecv.enableIRIn(); // 启动红外解码
    Serial.println("Initialisation complete.");
  
   
    while(!Serial);
 
    pinMode(sensorPin_1, INPUT);
   //ESP8266初始化
    Serial.print("setup begin\r\n");   

  Serial.print("FW Version:");
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStationSoftAP()) {
    Serial.print("to station + softap ok\r\n");
  } else {
    Serial.print("to station + softap err\r\n");
  }

  if (wifi.joinAP(SSID, PASSWORD)) {      //加入无线网
    Serial.print("Join AP success\r\n");  
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }

  if (wifi.disableMUX()) {
    Serial.print("single ok\r\n");
  } else {
    Serial.print("single err\r\n");
  }

  Serial.print("setup end\r\n");
    
   
}
void loop(void)     //循环函数  
{    
  if (irrecv.decode(&results)){
  
     
 if(results.value==33456255)
 {  for(int i=0;i<10;i++)  
  {Serial.println("上传");
if (sensor_time > millis())  sensor_time = millis();  
    
  if(millis() - sensor_time > INTERVAL_SENSOR)              //传感器采样时间间隔  
  {  
    getSensorData();                                        //读串口中的传感器数据
    sensor_time = millis();
  }  

    
  if (net_time1 > millis())  net_time1 = millis();
  
  if (millis() - net_time1 > INTERVAL_NET)                  //发送数据时间间隔
  {                
    updateSensorData();                                     //将数据上传到服务器的函数
    net_time1 = millis();
  
  }}}
   irrecv.resume();
  
  Serial.println(results.value);
} 
 
}

void getSensorData(){  
    sensor_tem = SHT2x.readT() ;   
    sensor_hum = SHT2x.readRH();   
    
   
    delay(1000);
    dtostrf(sensor_tem, 2, 1, sensor_tem_c);
    dtostrf(sensor_hum, 2, 1, sensor_hum_c);
    
}
void updateSensorData() {
  if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
    Serial.print("create tcp ok\r\n");

jsonToSend="{\"Temperature\":";
    dtostrf(sensor_tem,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    jsonToSend+=",\"Humidity\":";
    dtostrf(sensor_hum,1,2,buf);
    jsonToSend+="\""+String(buf)+"\"";
    
    jsonToSend+="}";



    postString="POST /devices/";
    postString+=DEVICEID;
    postString+="/datapoints?type=3 HTTP/1.1";
    postString+="\r\n";
    postString+="api-key:";
    postString+=apiKey;
    postString+="\r\n";
    postString+="Host:api.heclouds.com\r\n";
    postString+="Connection:close\r\n";
    postString+="Content-Length:";
    postString+=jsonToSend.length();
    postString+="\r\n";
    postString+="\r\n";
    postString+=jsonToSend;
    postString+="\r\n";
    postString+="\r\n";
    postString+="\r\n";

  const char *postArray = postString.c_str();                 //将str转化为char数组
  Serial.println(postArray);
  wifi.send((const uint8_t*)postArray, strlen(postArray));    //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
  Serial.println("send success");   
     if (wifi.releaseTCP()) {                                 //释放TCP连接
        Serial.print("release tcp ok\r\n");
        } 
     else {
        Serial.print("release tcp err\r\n");
        }
      postArray = NULL;                                       //清空数组，等待下次传输数据
  
  } else {
    Serial.print("create tcp err\r\n");
  }
  
}
