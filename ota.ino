/*
 * OTA functions. 
 * The ticker sets a boolena
 */
#include <ArduinoJson.h>

void updateCheckCB() {
  doUpdateCheck = true;
}
void updateCheckDo() {
  if(doUpdateCheck) {
    doUpdateCheck = false;
    char temp[128];
    Serial.println("OTA: Going to update firmware...");
    if((WiFiMulti.run() == WL_CONNECTED)) {
      Serial.println("OTA: Checking for Update. Current version: " + buildTag);
      Serial.println("OTA: URL " + checkUrl + "?firmware=mqttswitch2.0&version=" + buildTag);
      t_httpUpdate_return ret = ESPhttpUpdate.update(checkUrl + "?firmware=mqttswitch2.0&version=" + buildTag);
      switch(ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("OTA: HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("OTA: HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          Serial.println("OTA: HTTP_UPDATE_OK");
          DynamicJsonBuffer jsonBufferResponse;
          JsonObject& jsonmsgresponse = jsonBufferResponse.createObject();
          JsonObject& datamsgresponse = jsonmsgresponse.createNestedObject((const char*)mqttcfg.client_id);
          datamsgresponse["OTAUPDATE"] = "UPDATED";
          jsonmsgresponse.printTo(temp,sizeof(temp));
          MQTT_Publish(&mqttClient, statustopic, temp, os_strlen(temp), 2, 0);
          break;
      }
    }
  }
}
