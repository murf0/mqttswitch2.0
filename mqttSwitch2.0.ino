#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <Ticker.h>

/*
   FIX The SDK
   edit: ~/Library/Arduino15/packages/esp8266/hardware/esp8266/2.3.0/platform.txt  "-lssl -z muldefs" to the end
   symlin in Mqtt library ln -s $(pwd)/lib/mqtt ~/Documents/Arduino/libraries/mqtt

*/

//flag for saving data
bool shouldSaveConfig = false;

//Settings
//External connection which GPIO is connected
const int external_btn = 0;
const int external_relay = 2;

//Define OTA Settings
#define BUILD_TAG 0.0.2
#define CHECK_INTERVAL 60
String checkUrl = "http://192.168.0.156/esp/firmware.php";

//Global Variables
#define TEXTIFY(A) #A
#define ESCAPEQUOTE(A) TEXTIFY(A)
String buildTag = ESCAPEQUOTE(BUILD_TAG);


ESP8266WiFiMulti WiFiMulti;
Ticker updateCheck;
boolean doUpdateCheck = true;

void setup_gpio() {
  pinMode(external_relay, OUTPUT);
  digitalWrite(external_relay, LOW);
  pinMode(external_btn, INPUT_PULLUP);
}


/*
 * Setup function, Here we initialize Wifimanager, Gpios, MQTT connections and finally set up the OTA check
 */
void setup() {
  Serial.begin(115200);
  Serial.println();
  setup_wifi();
  setup_gpio();
  init_mqtt();
  updateCheck.attach(CHECK_INTERVAL, updateCheckCB);

  Serial.print("ESP: Free sketchspace: ");
  Serial.println(ESP.getFreeSketchSpace());
}

void loop() {
  //See ota.ino for OTA functions.
  updateCheckDo();
  //Nothing More to do here... 
}

