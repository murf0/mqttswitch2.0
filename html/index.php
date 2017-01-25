<!DOCTYPE html>
<html lang="en">
<head><meta http-equiv="Content-Type" content="text/html; charset=utf-8">

<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
<title>Home Automation</title>
<link href='https://fonts.googleapis.com/css?family=Josefin+Sans:400' rel='stylesheet' type='text/css'>
<style type="text/css">
    body {
        background-color: #eee;
    }
    .main {
        font-family: 'Josefin Sans', Arial, sans-serif;
        font-size: 82px;
        font-weight: 100;
        color: #777;
        text-align: center;
        padding-top: 50px;
        letter-spacing: 3px;
    }
    .mini {
        font-size: 12px;
        text-align: left;
        display: inline-block;
    }

    .formLayout {
        font-size: 12px;
        text-align: left;
        display: inline-block;
        background-color: #f3f3f3;
        border: solid 1px #a1a1a1;
        padding: 20px;
        width: 500px;
    }

    .formLayout button{
        display: block;
        width: 100px;
        float: left;
        margin-bottom: 5px;
        position: relative;
        top: 50%;
        transform: translateY(-50%);
    }

    .nicename {
        padding-right: 5px;
        padding-left: 5px;
        text-align: right;
        display: block;
        width: 250px;
        float: left;
        margin-bottom: 5px;
        position: relative;
        top: 50%;
        transform: translateY(-50%);
    }
    .gpio{
        padding-right: 5px;
        padding-left: 5px;
        text-align: right;
        width: 60px;
        display: block;
        float: left;
        margin-bottom: 5px;
        position: relative;
        top: 50%;
        transform: translateY(-50%);
    }
    br {
        clear: left;
    }
</style>
<!--[if IE]><script src="http://html5shiv.googlecode.com/svn/trunk/html5.js"></script><![endif]-->
    <script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js" type="text/javascript"></script>
    <script type="text/javascript" src="lib/mqttws31.js"></script>
    <script type="text/javascript" src="config.js"></script>
    <script type="text/javascript">
        function  nicenameDevice(text) {
            switch(text) {
                case "009C038D":
                    return "TV - Sovrum";
                    break;
                case "00FE466C":
                    return "Lampa - Vardagsrum";
                    break;
                case "009C022C":
                    return "Lampa - Kontor";
                    break;
                case "009C01E3":
                    return "Lampa - Vardagsrumsfönster";
                    break;
                case "000D930F":
                    return "Lampa - Sängbord M";
                    break;
                case "0009E479":
                    return "TV - Vardagsrum";
                    break;
                default:
                    return text;

            }
        }
        //MQTT Stuff
        function MQTTConnect() {
            mqtt = new Paho.MQTT.Client(host,port,"web_" + parseInt(Math.random() * 100,10));
            var options = {
                timeout: 3,
                useSSL: useTLS,
                cleanSession: cleansession,
                onSuccess: onConnect,
                onFailure: onFailure,
                userName: username,
                password: password
            };

            mqtt.onConnectionLost = onConnectionLost;
            mqtt.onMessageArrived = onMessageArrived;
            console.log("Host="+ host + ", port=" + port + " TLS = " + useTLS + " username=" + username + " password=" + password);
            mqtt.connect(options);
        }
        function onMessageArrived(message) {
          console.log("onMessageArrived:"+message.payloadString);
          if(debug === "true") {
            document.getElementById('STATUS').innerHTML = message.payloadString + "<br />" + document.getElementById('STATUS').innerHTML;
          }
          //check to see if payload is JSON
          try {
            if(!!document.getElementById("BUTTONS").innerHTML) {
              var BR="";
              console.log("FIRST")
            } else {
              var BR="<br \>";
            }
            var NO_OUTPUT = "false";
            var msgjson = jQuery.parseJSON(message.payloadString); //parse payload
            var NOTFIRST = "false";
            for (var key in msgjson) {
              var device=key;
              var output = "<div id=\"" + nicenameDevice(key) + "\" class=\"message\"><div class=\"nicename\">"
              output = output + nicenameDevice(key) + ": </div>";
              if(debug === "true") {
                output = output + "<button style=\"width: 20px; text-align:center; vertical-align:middle\" onclick='publish(\"{\\\""+device+"\\\":{\\\"Adress\\\":\\\"iot/switch/"+device+"/set\\\",\\\"Capability\\\":\\\"OFFLINE\\\"}}\",\"iot/switch/"+device+"/status\",2,true);'>-</button>";
              }
              console.log("DEVICE:" + device);
              var adress=msgjson[key]["Adress"];
              console.log("Adress:" + adress);
              //Check if the message is on or off status message.
              for (var OL in msgjson[key]) {
                if(msgjson[key][OL] === "ON") {
                                NO_OUTPUT = "true";
                                console.log("ON");
                                document.getElementById(key+"_OFF").style.backgroundColor="#FFFFFF";
                                document.getElementById(key+"_ON").style.backgroundColor="#00cc00";
                            } else if (msgjson[key][OL] === "OFF") {
                  NO_OUTPUT = "true";
                  console.log("OFF");
                                document.getElementById(key+"_ON").style.backgroundColor="#FFFFFF";
                                document.getElementById(key+"_OFF").style.backgroundColor="#ff0000";
                }
              }
              //check if the device is offline (LWT message from prev connected device)
              if(msgjson[key]["Capability"] === "OFFLINE") {
                NO_OUTPUT = "true";
                console.log("OFFLINE");
              }
              for (var key2 in msgjson[key]["Capability"]) {
                var GPIO=msgjson[key]["Capability"][key2];
                            //Request Status Message
                if(NOTFIRST==="true") {
                  output = output + "<label class=\"nicename\"></label>";
                }
                if(debug === "true") {
                  output = output + "<label class=\"gpio\">" + key2 +"</label>"
                }
                output = output + "<div><div class=\"buttonss\"><button id=\"" + key + "_ON\" onclick='publish(\"{\\\""+key2+"\\\":\\\"ON\\\"}\",\""+adress+"\",2,false);'>ON</button>";
                output = output + "<button id=\"" + key + "_OFF\" onclick='publish(\"{\\\""+key2+"\\\":\\\"OFF\\\"}\",\""+adress+"\",2,false);'>OFF</button></div></div>";

                NOTFIRST="true";
              }
              if(NOTFIRST==="false") {
                output = BR + output;
              }
              output = output + "</div>";
              //IF LWT or status message do not change the buttons.
              if(NO_OUTPUT !== "true") {
                document.getElementById("BUTTONS").innerHTML = output + document.getElementById("BUTTONS").innerHTML;

                console.log(output);
                            publish("{\""+key2+"\":\"STATUS\"}",adress,2,false);
              }
            }
          } catch (e) {
            console.log(e);
            return false;
          };

        }
        function onConnect() {
            // Once a connection has been made, make a subscription and send a message.
            document.getElementById("BUTTONS").innerHTML = "";
            console.log("onConnect Sub to: iot/switch/+/status");
            mqtt.subscribe("iot/switch/+/status", {qos:1,onSuccess:onSubscribe,onFailure:onSubscribeFailure});
        };
        function onSubscribe(x) {
          console.log('Subscribed');
        }
        function onSubscribeFailure(x) {
          console.log('Subscribe failed: ' + message.errorMessage);
        }
        function onFailure(responseObject) {
            console.log("onFailure: " + responseObject.errorMessage + "Retrying");
            setTimeout(MQTTConnect, reconnectTimeout);
        }
        function onConnectionLost(responseObject) {
            document.getElementById("BUTTONS").innerHTML = "<br />Connection Lost<br />";
          if (responseObject.errorCode !== 0)
            console.log("onConnectionLost:"+responseObject.errorMessage);
            setTimeout(MQTTConnect, reconnectTimeout);
        }
        function getURLParameters(paramName) {
            var sURL = window.document.URL.toString();
            if (sURL.indexOf("?") > 0)
            {
               var arrParams = sURL.split("?");
               var arrURLParams = arrParams[1].split("&");
               var arrParamNames = new Array(arrURLParams.length);
               var arrParamValues = new Array(arrURLParams.length);
               var i = 0;
               for (i=0;i<arrURLParams.length;i++)
               {
                var sParam =  arrURLParams[i].split("=");
                arrParamNames[i] = sParam[0];
                if (sParam[1] != "")
                    arrParamValues[i] = unescape(sParam[1]);
                else
                    arrParamValues[i] = "No Value";
               }

               for (i=0;i<arrURLParams.length;i++)
               {
                        if(arrParamNames[i] == paramName){
                    //alert("Param:"+arrParamValues[i]);
                        return arrParamValues[i];
                     }
               }
               return null;
            }

        }
        var publish = function (payload, topic, qos, retained) {
            console.log("Topic: " + topic + " Publish: " + payload);
            var message = new Paho.MQTT.Message(payload);
            message.destinationName = topic;
            message.qos = qos;
            message.retained = retained;
            mqtt.send(message);
        }

        //Start The Goddamn Program
        var username = getURLParameters("username");
        var password = getURLParameters("password");
        var debug = getURLParameters("debug");
        MQTTConnect();
    </script>
  </head>
<body id="index" class="home">
    <div class="main">
        Home Automation<br />
        <div class="formLayout" id="BUTTONS"></div>
        <br />
        <div class="mini">
            <label id="STATUS"></label>
        <br />
        mikael@murf.se 2017
        </div>
    </div>
</body>
</html>