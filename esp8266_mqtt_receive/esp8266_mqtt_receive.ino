//receive messages over mqtt
//also toggle on board led by sending on/off on topic led

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED 2

const char* SSID = "KabelBox-E84E";
const char* PSK = "67534404306756752453";
const char* MQTT_BROKER = "192.168.0.7";
#define MQTT_USER "esp8266bme280"
#define MQTT_PASSWD "4liveweather"

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    setup_wifi();
    client.setServer(MQTT_BROKER, 1883);
    client.setCallback(callback);
}

void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(SSID);
 
    WiFi.begin(SSID, PSK);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Received message [");
    Serial.print(topic);
    Serial.print("] ");
    char msg[length+1];
    for (int i = 0; i < length; i++) {
        //Serial.print((char)payload[i]);
        msg[i] = (char)payload[i];
    }
    Serial.println();
    
    msg[length] = '\0';
    Serial.println(msg);

    if(strcmp(topic,"led")==0){
      if(strcmp(msg,"on")==0){
          digitalWrite(LED, LOW);
      }
      else if(strcmp(msg,"off")==0){
          digitalWrite(LED, HIGH);
      }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("Reconnecting MQTT...");
        if (!client.connect("ESP8266Client",MQTT_USER,MQTT_PASSWD)) {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
    client.subscribe("#");
    Serial.println("MQTT Connected...");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}
