/*
weather logger v2
running on battery
save power by:
*remove linear regulator
*save wifi channel & power off wifi
*static ip
*power off bme280
*/

/*
 * connections
 * D0 - RST (for wake up)
 * D1 - SCL
 * D2 - SDA
 * GND, VCC (3.3V)
*/

/*
debug mosquitto:
sudo systemctl stop mosquitto
sudo mosquitto -v
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "cactus_io_BME280_I2C.h"

//network credentials
#define WLAN_SSID "..."
#define WLAN_PASSWD "..."
#define MQTT_BROKER "..."
#define MQTT_USER "..."
#define MQTT_PASSWD "..."
#define SLEEPTIME 300e6 //300s sleep

//BME280
#define TEMP_OFFSET 0
#define BME280_POWER D4

//wifi parameters
//for mac: 84:CC:A8:84:66:78
IPAddress local_IP(192, 168, 0, 16);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);

//MQTT instance
WiFiClient espClient;
PubSubClient client(espClient);

//4 bytes aligned data struct on rtc memory (stored even when in deep sleep) for crc and wifi channel/mac
struct {
  uint32_t crc32;   // 4 bytes
  uint8_t channel;  // 1 byte,   5 in total
  uint8_t ap_mac[6]; // 6 bytes, 11 in total
  uint8_t padding;  // 1 byte,  12 in total
} rtcData;

//flags when received sent data => data gets successfuly send at least once (mqtt quality of service: 1)
bool p = true;
bool h = true;
bool t = true;

void setup() {
  //serial print for debug
//  Serial.begin(115200);
//  Serial.println();
  
  //power to bme280
  pinMode(BME280_POWER,OUTPUT);
  digitalWrite(BME280_POWER,HIGH);
  //start BME280
  BME280_I2C bme(0x76);
  bme.begin();
  bme.setTempCal(TEMP_OFFSET);
  
  //check rtc memory by comparing with previous crc checksum
  bool rtcValid = false;
  if( ESP.rtcUserMemoryRead( 0, (uint32_t*)&rtcData, sizeof( rtcData ) ) ) {
    uint32_t crc = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    if( crc == rtcData.crc32 ) {
      rtcValid = true;
//      Serial.println("rtc valid");
    }
  }
  
  //access wifi
  WiFi.config(local_IP, gateway, subnet);
  if( rtcValid ) {
    WiFi.begin( WLAN_SSID, WLAN_PASSWD, rtcData.channel, rtcData.ap_mac, true );
//    Serial.println("presaved wifi connect");
  }
  else {
    WiFi.begin( WLAN_SSID, WLAN_PASSWD );
//    Serial.println("arbitrary wifi connect");
  }
  delay( 5 );
  
  //connect
//  Serial.print("Connecting to Wifi: ");
//  Serial.println(WLAN_SSID);
  int retries = 0;
  while( WiFi.status() != WL_CONNECTED ) {
    retries++;
//    Serial.print(".");
    if( retries > 40 ) {
      // Giving up after 20 retries and going back to sleep
      power_off();
    }
    if( retries == 10 ) {
      // Quick connect is not working, reset WiFi and try regular connection
//      Serial.println("arbitrary wifi connect");
      WiFi.begin( WLAN_SSID, WLAN_PASSWD );
    }
    delay( 500 );
  }
  
//  Serial.println("");
//  Serial.println("WiFi connected");
//  Serial.print("IP address: ");
//  Serial.println(WiFi.localIP());
//  Serial.print("router MAC address: ");
//  char bssid[6];
//  memcpy( bssid, WiFi.BSSID(), 6 );
//  Serial.println(bssid);
//  Serial.print("esp8266 MAC address: ");
//  Serial.println(WiFi.macAddress());
  
  //connect to MQTT server
//  Serial.println("connect to mqtt server");
  client.setServer(MQTT_BROKER, 1883);
  client.setCallback(callback);
  retries = 0;
  while (!client.connected()) {
    retries++;
    if (retries>20) {
      power_off();
    }
    if (!client.connect("ESP8266Client",MQTT_USER,MQTT_PASSWD)) {
      delay( 500 );
//      Serial.println("retry connecting to mqtt server");
    }
  }
  client.loop();
  
/*  Serial.println("-----RTC before write-----");
  Serial.print("wifi ch: ");
  Serial.println(rtcData.channel);
  Serial.println(WiFi.channel());
  Serial.print("crc: ");
  Serial.println(rtcData.crc32);
  Serial.println(calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 ));
  Serial.print("ap_mac: ");
  char bssid1[6]; char bssid2[6];
  memcpy( bssid1, rtcData.ap_mac, 6 );
  Serial.println(bssid1);
  memcpy( bssid2, WiFi.BSSID(), 6 );
  Serial.println(bssid2);
  Serial.print("rtcValid: ");
  Serial.println(rtcValid);*/
  
  // Write current connection info back to RTC
//  Serial.println("all successful, save wifi credentials");
  rtcData.channel = WiFi.channel();
  memcpy( rtcData.ap_mac, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
  rtcData.crc32 = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
  ESP.rtcUserMemoryWrite( 0, (uint32_t*)&rtcData, sizeof( rtcData ) );
  
/*  Serial.println("-----RTC after write-----");
  Serial.print("wifi ch: ");
  Serial.println(rtcData.channel);
  Serial.println(WiFi.channel());
  Serial.print("crc: ");
  Serial.println(rtcData.crc32);
  Serial.println(calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 ));
  Serial.print("ap_mac: ");
  //char bssid1[6]; char bssid2[6];
  memcpy( bssid1, rtcData.ap_mac, 6 );
  Serial.println(bssid1);
  memcpy( bssid2, WiFi.BSSID(), 6 );
  Serial.println(bssid2);
  Serial.print("rtcValid: ");
  Serial.println(rtcValid);*/
  
  //BME280 data
//  Serial.println("read and publish sensor data");
  char pres[10];
  char humi[10];
  char temp[10];
  bme.readSensor();
  float pressure = bme.getPressure_MB();
  float humidity = bme.getHumidity();
  float temperature = bme.getTemperature_C();
//  Serial.println(pressure);
//  Serial.println(humidity);
//  Serial.println(temperature);
  digitalWrite(BME280_POWER,LOW);
  dtostrf(pressure,7, 2, pres);
  dtostrf(humidity,7, 2, humi);
  dtostrf(temperature,7, 2, temp);
  client.subscribe("/weather/pressure");
  client.subscribe("/weather/humidity");
  client.subscribe("/weather/temperature");
//  Serial.println("start sending data in a loop");
  int count = 0;
  while(p){count+=1;if(count>10){break;};client.publish("/weather/pressure", (char*) pres); for(uint8_t i=0;i<1000;i++){client.loop(); delayMicroseconds(20);if(!p){break;}}}
  count = 0;
  while(h){count+=1;if(count>10){break;};client.publish("/weather/humidity", (char*) humi); for(uint8_t i=0;i<1000;i++){client.loop(); delayMicroseconds(20);if(!h){break;}}}
  count = 0;
  while(t){count+=1;if(count>10){break;};client.publish("/weather/temperature", (char*) temp); for(uint8_t i=0;i<1000;i++){client.loop(); delayMicroseconds(20);if(!t){break;}}}
  
  delay( 2 );
//  Serial.println("done everything, go to sleep");
  power_off();
}

//process subscribed messages: test if send message returned
void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,"/weather/pressure")==0){p = false;}
  else if(strcmp(topic,"/weather/humidity")==0){h = false;}
  else if(strcmp(topic,"/weather/temperature")==0){t = false;}
}

//calculateCRC32()
uint32_t calculateCRC32( const uint8_t *data, size_t length ) {
  uint32_t crc = 0xffffffff;
  while( length-- ) {
    uint8_t c = *data++;
    for( uint32_t i = 0x80; i > 0; i >>= 1 ) {
      bool bit = crc & 0x80000000;
      if( c & i ) {
        bit = !bit;
      }
      crc <<= 1;
      if( bit ) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

void power_off(void){
//  Serial.println("power off");
  digitalWrite(BME280_POWER,LOW);
  WiFi.disconnect( true );
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  //wait untill wifi disconnected
  int count = 0;
  while(WiFi.status() == WL_CONNECTED)
  {if(count>50){break;}delay(1);count++;}
  ESP.deepSleep( SLEEPTIME, WAKE_RF_DEFAULT);
  //ESP.deepSleep( SLEEPTIME, WAKE_RF_DISABLED); //danger: diables Wifi !!!
  //ESP.deepSleep(1, WAKE_RF_DEFAULT); //enable wifi after deep sleep //danger: stuck in loop !!!
}

void loop(){}
