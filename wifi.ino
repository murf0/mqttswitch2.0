#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include "private.h"

void saveConfigCallback () {
  Serial.println("WIFI: Should save config");
  shouldSaveConfig = true;
}

void setup_wifi() {
  char mqtt_server[64];
  char mqtt_username[32];
  char mqtt_password[32];
  char mqtt_port[6];
  
    //clean FS, for testing
  //SPIFFS.format();
  //read configuration from FS json
  Serial.println("WIFI: mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("WIFI: mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("WIFI: reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("WIFI: opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nWIFI: parsed json");

          strcpy(mqtt_server, json["mqtt_server"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_username, json["mqtt_username"]);
          strcpy(mqtt_password, json["mqtt_password"]);
        } else {
          Serial.println("WIFI: failed to load json config");
        }
      }
    }
  } else {
    Serial.println("WIFI: failed to mount FS");
  }
  //end read


  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "mqtt Server", "MQTT_SERVER", 64);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "MQTT_PORT", 6);
  WiFiManagerParameter custom_mqtt_username("username", "mqtt Username", "MQTT_USER", 32);
  WiFiManagerParameter custom_mqtt_password("password", "mqtt Password", "MQTT_PASSWORD", 32);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);
  //reset settings - for testing
  //wifiManager.resetSettings();
  
  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("mqttSwitch2.0", "password")) {
    Serial.println("WIFI: failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("WIFI: Connected wifi");

  //read updated parameters
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());
  strcpy(mqtt_username, custom_mqtt_username.getValue());
  strcpy(mqtt_password, custom_mqtt_password.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("WIFI: saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_server"] = mqtt_server;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_username"] = mqtt_username;
    json["mqtt_password"] = mqtt_password;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("WIFI: failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }
  Serial.print("WIFI: local ip ");
  Serial.println(WiFi.localIP());

  os_sprintf((char*) mqttcfg.mqtt_host, "%s", mqtt_server);
  char *ptr;
  mqttcfg.mqtt_port=strtol(mqtt_port, &ptr, 10);
  os_sprintf((char*) mqttcfg.mqtt_user, "%s", mqtt_username);
  os_sprintf((char*) mqttcfg.mqtt_pass, "%s", mqtt_password);
}
