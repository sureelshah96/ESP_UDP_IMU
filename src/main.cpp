#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncUDP.h>
#include <NTPClient.h>
#include <WiFiUdp.h>


const char * ssid = "Primary";
const char * pass = "71017101";
const char *on = "1";
const char *off = "0";
static int count = 0;

int LED1 = 2;      // Assign LED1 to pin GPIO2

int LED2 = 16;     // Assign LED1 to pin GPIO16


// const char * addr = "192,168,137,1";
// const uint32_t addr = ("192,168,137,1");
String addr = String("192,168,137,1");
// long unsigned int *addr = "192.168.137.1";

#define NTP_OFFSET   60*60*4      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "0.us.pool.ntp.org"

// I am using 4 Sliders the 5th one is not used
const int NUM_SLIDERS = 5;
// I am using a NODEMCU ESP 32S and pins SVP, SVN, P34, P35, (P32 (Not used))
const int analogInputs[NUM_SLIDERS] = {36, 39, 34, 35, 32};

int analogSliderValues[NUM_SLIDERS];

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
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    

    if(udp.listen(IPAddress(192,168,137,195),1000)) 
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
            }
            // else if(strcmp(data, off) == 0)
            else if(*data==*off)
            {
              digitalWrite(LED1, LOW);
            }
            
            //   delay(200);
            // digitalWrite(LED2, LOW);



        });
    }
    timeClient.begin();

  pinMode(LED1, OUTPUT);

  pinMode(LED2, OUTPUT);
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
  
  String builtString = (String("ESP: 2  Time: "+timeClient.getFormattedTime()+" ID:"+count));
  
  if (count>1000)
  {
    count=0;
  }
  // String builtString = String("");

  // for (int i = 0; i < NUM_SLIDERS; i++) {
  //   builtString += String((int)analogSliderValues[i]);
  //   // Build the string to broadcast by seperating values using | except for last value
  //   if (i < NUM_SLIDERS - 1) {
  //     builtString += String("|");
  //   }
  // }
  // Send UDP Broadcast to 255.255.255.255 (default broadcast addr), Port 2255
  // Serial.println(addr.c_str());
  udp.writeTo((uint8_t *)builtString.c_str(), strlen(builtString.c_str()) , IPAddress(192,168,137,1) , (int)2255);
}

void loop()
{
    // updateSliderValues();
    timeClient.update();
    sendSliderValues(); // Send data
    delay(1);
}