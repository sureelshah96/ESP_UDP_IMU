#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include "I2Cdev.h"
#include <ArduinoOTA.h>



const char * ssid = "ESP";
const char * pass = "password";
const char *on = "1";
const char *off = "0";
static int count = 0;

int LED1 = 2;      // Assign LED1 to pin GPIO2

int LED2 = 4;     // Assign LED2 to pin GPIO4, Used for STM interrupt

unsigned long Time = 0;
unsigned long old_Time = 0;

int Frequency = 1;  //Hz
float transmission_rate = 1000/Frequency;



String addr = String("192,168,137,1");

#define NTP_OFFSET   60*60*4      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "0.us.pool.ntp.org"




AsyncUDP udp;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);


void setup()
{
   
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    
    ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("OTA End");
      Serial.println("Rebooting...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    
    
    
    if(udp.listen(IPAddress(192,168,1,4),1000)) 
    {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
          
            char *data = (char*)packet.data();


            if(*data==*on)
            {
              digitalWrite(LED1, HIGH);  //Onboard LED
              digitalWrite(LED2, LOW);
            }
            else if(*data==*off)
            {
              digitalWrite(LED1, LOW);
              digitalWrite(LED2, HIGH);
            }

        });
    }
    timeClient.begin();

  pinMode(LED1, OUTPUT);

  pinMode(LED2, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  

}


void sendSliderValues() {
  count+=1;
  
  String builtString = (String("ESP: 4  Time: "+timeClient.getFormattedTime()+" ID:"+count));
  
  if (count>1000)
  {
    count=0;
  }

 
  udp.writeTo((uint8_t *)builtString.c_str(), strlen(builtString.c_str()) , IPAddress(192,168,1,3) , (int)2255);
}

void loop()
{
  ArduinoOTA.handle();
  Time = millis();
  // timeClient.update();
  if ((Time-old_Time) >= transmission_rate)
  {
    Serial.println(Time-old_Time);
    sendSliderValues();
    old_Time = millis();
  }
  
}

