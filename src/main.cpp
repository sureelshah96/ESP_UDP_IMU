#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include "I2Cdev.h"
#include <ArduinoOTA.h>


// #define OUTPUT_READABLE_QUATERNION
// MPU6050 mpu;


const char * ssid = "ESP";
const char * pass = "password";
const char *on = "1";
const char *off = "0";
static int count = 0;

int LED1 = 2;      // Assign LED1 to pin GPIO2

int LED2 = 4;     // Assign LED1 to pin GPIO16

unsigned long Time = 0;
unsigned long old_Time = 0;

int Frequency = 1;
float transmission_rate = 1000/Frequency;


// const char * addr = "192,168,137,1";
// const uint32_t addr = ("192,168,137,1");
String addr = String("192,168,137,1");
// long unsigned int *addr = "192.168.137.1";

#define NTP_OFFSET   60*60*4      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "0.us.pool.ntp.org"

// WiFiServer TelnetServer(2255);



AsyncUDP udp;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);


void setup()
{
    // Set all slider pins to INPUT
    // for (int i = 0; i < NUM_SLIDERS; i++) {
    //   pinMode(analogInputs[i], INPUT);
    // }
    // Get WiFi going
    // Wire.begin();
    // Wire.setClock(400000);


    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    
    // mpu.initialize();
    // Serial.println(F("Testing device connections..."));
    // Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
    
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
            // Serial.print("UDP Packet Type: ");
            // Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print("From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            // Serial.print(", To: ");
            // Serial.print(packet.localIP());
            // Serial.print(":");
            // Serial.print(packet.localPort());
            // Serial.print(", Length: ");
            // Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
            //reply to the client
            // packet.printf("Got %u bytes of data", packet.length());
            char *data = (char*)packet.data();

            // Serial.println(*data);            
            // Serial.println(*on);


            // if(strcmp(data,on) == 0)
            if(*data==*on)
            {
              digitalWrite(LED1, HIGH);
              digitalWrite(LED2, LOW);
            }
            // else if(strcmp(data, off) == 0)
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


// Get slider pin values
// void updateSliderValues() {
//   for (int i = 0; i < NUM_SLIDERS; i++) {
//      analogSliderValues[i] = analogRead(analogInputs[i]);
//   }
// }

// TODO only send values if there is significant change in values to limit network traffick
// UDP Broadcast slider values
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
    // updateSliderValues();
  ArduinoOTA.handle();
  Time = millis();
  // timeClient.update();
  if ((Time-old_Time) >= transmission_rate)
  {
    Serial.println(Time-old_Time);
    sendSliderValues();
    old_Time = millis();
  }
  // sendSliderValues(); // Send data
  // delay(10);
}

