#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <c_types.h>
#include <os_type.h>
#include <mem.h>

extern "C" {
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "cont.h"
#include "eagle_soc.h"
#include "espconn.h"
void *pvPortZalloc(size_t, char * file, int line);
} 

typedef boolean BOOL;
#include <mqtt.h>

typedef struct{
    uint8_t mqtt_host[64];
    uint32_t mqtt_port;
    uint8_t mqtt_user[32];
    uint8_t mqtt_pass[32];
    uint8_t client_id[16];
    uint32_t mqtt_keepalive;
    uint8_t security;
} MQTTCFG;

MQTTCFG mqttcfg;

char subtopic[64];
char statustopic[64];
char registertopic[64];
int GPIO2_status = 0;
int GPIO13_status = 0;

MQTT_Client mqttClient;
int RESET=0;

/*Set up debug messages*/
#define dbg 1


/*#define CAPABILITY "{\"%s\":{\"Adress\":\"%s\",\"Capability\":{\"GPIO02\": \"true\",\"GPIO13\": \"true\"}}}"*/
#define CAPABILITY "{\"%s\":{\"Adress\":\"%s\",\"Capability\":{\"GPIO02\": \"true\"}}}"

// Button Press debouncing
// Holds the last time debounce was evaluated (in millis).
volatile long lastDebounceTime = 0;
// The delay threshold for debounce checking.
const int debounceDelay = 500;

void btnPressed() {
  //Serial.println("BTN: btnPressed (Before Debounce check)");
  //Do Debouncing
  boolean debounce = false;
  // Check to see if the change is within a debounce delay threshold.
  if((millis() - lastDebounceTime) <= debounceDelay) {
    debounce = true;
  }
  lastDebounceTime = millis();
  if(debounce) {
    return; 
  }
  //USING npn 2N3904
  char temp[128];
  //Serial.println("BTN: btnPressed");
  DynamicJsonBuffer jsonBuffer2;
  JsonObject& jsonmsgresponse = jsonBuffer2.createObject();
  JsonObject& data = jsonmsgresponse.createNestedObject((const char*)mqttcfg.client_id);
  #ifdef NPN
    if(digitalRead(external_relay) == LOW) {
      pinMode(external_relay, INPUT);
      data["GPIO02"] = "OFF";
    } else {
      pinMode(external_relay, OUTPUT);
      digitalWrite(external_relay,LOW);
      data["GPIO02"] = "ON";
    }
    
  #else
    if(digitalRead(external_relay) == HIGH) {
      //Serial.println("BTN: Turning Relay OFF");
      digitalWrite(external_relay,LOW);
      data["GPIO02"] = "OFF";
    } else if(digitalRead(external_relay) == LOW) {
      //Serial.println("BTN: Turning Relay ON");
      digitalWrite(external_relay,HIGH);
      data["GPIO02"] = "ON";
    }
  #endif
  jsonmsgresponse.printTo(temp,sizeof(temp));
  MQTT_Publish(&mqttClient, statustopic, temp, os_strlen(temp), 2, 0);
  RESET++;
  Serial.println(RESET);
  if( RESET > 9) {
      WiFiManager wifiManager;
      if (!wifiManager.startConfigPortal("OnDemandAP")) {
          Serial.println("failed to connect and hit timeout");
          delay(3000);
          //reset and try again, or maybe put it to deep sleep
          ESP.reset();
          delay(5000);
      }  
  }
}

void mqttConnectedCb(uint32_t *args) {
    Serial.print("MQTT: mqttConnectedCb");
    MQTT_Client* client = (MQTT_Client*)args;
    char temp[128];
    char offline[128];
    DynamicJsonBuffer jsonBuffer;
    JsonObject& jsonmsgresponse = jsonBuffer.createObject();
    JsonObject& data = jsonmsgresponse.createNestedObject((const char*)mqttcfg.client_id);   
     
    os_sprintf(subtopic,"iot/switch/%s/set",mqttcfg.client_id);
    os_sprintf(statustopic,"iot/switch/%s/status",mqttcfg.client_id);
    os_sprintf(temp,CAPABILITY, mqttcfg.client_id,subtopic);
    
    data["Adress"] = subtopic;
    data["Capability"] = "OFFLINE";
    jsonmsgresponse.printTo(offline, sizeof(offline));
    //jsonmsgresponse.printTo(Serial);
    
    //MQTT_InitLWT(MQTT_Client *mqttClient, uint8_t* will_topic, uint8_t* will_msg, uint8_t will_qos, uint8_t will_retain);
    MQTT_InitLWT(client, (uint8_t*) statustopic, (uint8_t*) offline, (uint8_t) 2, (uint8_t)1);
    MQTT_Subscribe(client, subtopic, 2);
    Serial.print("MQTT: subscribed to ");
    Serial.println(subtopic);
    Serial.print("MQTT: Statustopic is ");
    Serial.println(statustopic);
    Serial.print("MQTT: Capability ");
    Serial.println(temp);
    Serial.print("MQTT: LWT payload ");
    Serial.println(offline);
    MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 1);
}


void mqttDisconnectedCb(uint32_t *args) {
  Serial.print("MQTT: mqttDisconnectedCb");
  MQTT_Client* client = (MQTT_Client*)args;
  Serial.println("MQTT: Disconnected");
}

void mqttPublishedCb(uint32_t *args) {
  Serial.print("MQTT: mqttPublishedCb");
  MQTT_Client* client = (MQTT_Client*)args;
  Serial.println("MQTT: Published");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len) {
   Serial.print("MQTT: mqttDataCb");
   char *topicBuf = (char*)os_zalloc(topic_len+1), *dataBuf = (char*)os_zalloc(data_len+1);
  
   MQTT_Client* client = (MQTT_Client*)args;
   int i;
   bool sendmsg=false;
   char *tmp_gpios, *status, *out;
   char temp[128];
    
   os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;
    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    Serial.print("MQTT: Received on topic: ");
    Serial.println(topicBuf);
    Serial.print("MQTT: Data:");
    Serial.println(dataBuf);
    /*PARSE MESSG
    Commands available is: 
    {\"GPIO02\":\"ON\"}
    {\"GPIO02\":\"OFF\"}
    {\"GPIO02\":\"STATUS\"}
    */

    DynamicJsonBuffer jsonBuffer;
    JsonObject& jsonmsg = jsonBuffer.parseObject(dataBuf);
    if (!jsonmsg.success()) {
      Serial.println("Parsing Failed");
    }
    DynamicJsonBuffer jsonBufferResponse;
    JsonObject& jsonmsgresponse = jsonBufferResponse.createObject();
    JsonObject& datamsgresponse = jsonmsgresponse.createNestedObject((const char*)mqttcfg.client_id);
    if(jsonmsg.containsKey("GPIO02")) {
      Serial.println("MQTT: Testing GPIO02 ON");
      if(strcmp(jsonmsg["GPIO02"],"ON")==0) {
          #ifdef NPN
            pinMode(external_relay, OUTPUT);
            digitalWrite(2,LOW);
          #else
            digitalWrite(2,HIGH);
          #endif
          datamsgresponse["GPIO02"] = "ON";
          Serial.println("MQTT: GPIO2 == ON");
          GPIO2_status = 1;
          sendmsg=true;
      }
      Serial.println("MQTT: Testing GPIO02 OFF");
      if(strcmp(jsonmsg["GPIO02"],"OFF")==0) {
          #ifdef NPN
            pinMode(external_relay, INPUT);
          #else
            digitalWrite(2,LOW);
          #endif
          datamsgresponse["GPIO02"] = "OFF";
          Serial.println("MQTT: GPIO2 == OFF");
          GPIO2_status = 0;
          sendmsg=true;
      }
      Serial.println("MQTT: Testing GPIO02 STATUS");
      if(strcmp(jsonmsg["GPIO02"],"STATUS")==0) {
          if(GPIO2_status == 0) {
            datamsgresponse["GPIO02"] = "OFF";
            Serial.println("MQTT: GPIO2 == OFF");
          } else {
            datamsgresponse["GPIO02"] = "ON";
            Serial.println("MQTT: GPIO2 == ON");
          }
          sendmsg=true;
      }
    }
    if(jsonmsg.containsKey("GPIO13")) {
      Serial.println("MQTT: Testing GPIO13 ON");
      if(strcmp(jsonmsg["GPIO13"],"ON")==0) {
          digitalWrite(13,HIGH);
          datamsgresponse["GPIO13"] = "ON";
          Serial.println("MQTT: GPI13 == ON");
          GPIO13_status = 1;
          sendmsg=true;
      }
      Serial.println("MQTT: Testing GPIO13 OFF");
      if(strcmp(jsonmsg["GPIO13"],"OFF")==0) {
          digitalWrite(13,LOW);
          datamsgresponse["GPIO13"] = "OFF";
          Serial.println("MQTT: GPI13 == OFF");
          GPIO13_status = 0;
          sendmsg=true;
      }
      Serial.println("MQTT: Testing GPIO013 STATUS");
      if(strcmp(jsonmsg["GPIO13"],"STATUS")==0) {
          if(GPIO13_status == 0) {
            datamsgresponse["GPIO13"] = "OFF";
            Serial.println("MQTT: GPI13 == OFF");
          } else {
            datamsgresponse["GPIO13"] = "ON";
            Serial.println("MQTT: GPI13 == ON");
          }
          sendmsg=true;
      }
    }
    if(sendmsg==true) {
      jsonmsgresponse.printTo(temp,sizeof(temp)); 
      MQTT_Publish(&mqttClient, statustopic, temp, os_strlen(temp), 2, 0);
    }

}

void mqttTimeoutCb(uint32_t *args) {
  Serial.print("MQTT: mqttTimeoutCb");
  // Since we timed out the MQTT login/or connection. Restart the AP and try again.
  MQTT_Client* client = (MQTT_Client*)args;
  //WiFiManager wifiManager;
  //wifiManager.resetSettings();
  ESP.restart();
}

void  init_mqtt(void) {
    os_sprintf((char*)mqttcfg.client_id, "%08X", system_get_chip_id());
    Serial.print("MQTT: ClientID ");
    Serial.println((char*)mqttcfg.client_id);
    
    mqttcfg.mqtt_keepalive=120;
    mqttcfg.security = 1; // 1=ssl (max 1024bit certificate) 0=nonssl
    Serial.print("MQTT: server ");
    Serial.println((char*) mqttcfg.mqtt_host);
    Serial.print("MQTT: Port ");
    Serial.println(mqttcfg.mqtt_port);
    Serial.print("MQTT: Username ");
    Serial.println((char*) mqttcfg.mqtt_user);
    Serial.print("MQTT: Password ");
    Serial.println((char*) mqttcfg.mqtt_pass);
    Serial.print("MQTT: Security ");
    Serial.println(mqttcfg.security);
    Serial.print("MQTT: ClientID ");
    Serial.println((char*)mqttcfg.client_id);
    Serial.print("MQTT: Keepalive ");
    Serial.println(mqttcfg.mqtt_keepalive);
    
    Serial.println("MQTT: MQTT_InitConnection");
    MQTT_InitConnection(&mqttClient, mqttcfg.mqtt_host, mqttcfg.mqtt_port, mqttcfg.security);
    Serial.println("MQTT: MQTT_InitClient");
    MQTT_InitClient(&mqttClient, mqttcfg.client_id, mqttcfg.mqtt_user, mqttcfg.mqtt_pass, mqttcfg.mqtt_keepalive, 1);
    Serial.println("MQTT: MQTT_OnConnected");
    MQTT_OnConnected(&mqttClient, mqttConnectedCb);
    Serial.println("MQTT: MQTT_OnDisconnected");
    MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
    Serial.println("MQTT: MQTT_OnPublished");
    MQTT_OnPublished(&mqttClient, mqttPublishedCb);
    Serial.println("MQTT; MQTT_OnData");
    MQTT_OnData(&mqttClient, mqttDataCb);
    Serial.println("MQTT: MQTT_OnTimeout");
    MQTT_OnTimeout(&mqttClient, mqttTimeoutCb);
    Serial.println("MQTT: MQTT_Connect");
    MQTT_Connect(&mqttClient);
      
  
    // Do button
    Serial.println("MQTT: Button attachInterrupt");
    attachInterrupt(external_btn, btnPressed, FALLING);
}

