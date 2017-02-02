**mqttSwitch2.0**
==========
**Features:**

ESP8266
* Status of GPIO pins as published mqtt-message
* Capability of esp sent on boot to mqtt broker
* Set status of GPIO pin (on-off for now) via MQtt Message (Set at compiletime capabilities)
* WifiManager with Mqtt settings available
* OTA prepared (For boards with more than 4mbit memory)
* Mangled version of [MQTT library for the ESP8266 ](https://github.com/tuanpmt/esp_mqtt) library included. Will now work with Arduino ESP8266 SDK. (after a slight mod to platform.txt)
* Mqtt with SSL enabled only
* Mqtt with username + password

HTML
* Java-script enabled interface to control multiple switches. Uses Getvariables for username/password
* Nice-name for each switch configured here
* No webserver required everything is handled in web-browser

OTA Firmwarecheck
* php required, script included based on the standard examples.

**Installation**
(Instructions for macOS)

* edit: ~/Library/Arduino15/packages/esp8266/hardware/esp8266/2.3.0 platform.txt  add "-lssl --allow-multiple-definition" to the end of the Linker row. (the row begins with compiler.c.elf.libs=)
See http://www.delorie.com/gnu/docs/binutils/ld_3.html for information. The linker uses the first found definition)
* symlink in Mqtt library "ln -s $(pwd)/lib/mqtt ~/Documents/Arduino/libraries/mqtt" or move it there
***Libraries used***
* [MQTT library for the ESP8266 ](https://github.com/tuanpmt/esp_mqtt) By Tuanpmt (thanks!) (This is included in the repo for use by Arduino ESP8266) (MIT-License)
* [WifiManager library for the ESP8266 ](https://github.com/tzapu/WiFiManager#install-through-library-manager) By Tzapu (MIT-License)
* [ArduinoJson library for Arduino](https://github.com/bblanchon/ArduinoJson) (MIT-License)

**Hardware**
(This description is a mess sorry, please see included eagle files)
GPIO2 is connected to the base of a NPN3906 transistor via a 1k resistor. Collector is conneted to Ground, Emittor is connected to the Relay. the Groound of relay to 5.0v (USB)
ESP8266 -03, GPIO 15 via 10k resistor to ground, CH_PD via 10k resistor to VCC 3.3v regulator AMS1117 connected to VCC5.0

***Schematic***
![Schematic](images/schematic.png "Schematic")

***Board***
![Board](images/board.png "Schematic")

**Author:**
[Mikael "Murf" Mellgren](https://murf.se)

**LICENSE - "BSD"**
mqttSwitch2.0: Copyright (c) 2017 Mikael MURF Mellgren , https://murf.se

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
