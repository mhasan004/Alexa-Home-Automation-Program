# Devices_Alexa_Program
This custom program to turn on/off a lamp and turn a servo a certain angle using Alexa, Raspberry Pi, and the ESP8266 through the MQTT protocol 

## FILES: ##
* **home_automation.py** is the python script that used Flask and Flask-Ask libraries to process Alexa intents. It will send servo angle values to the ESP8266 by publishing them to the MQTT server.
* **MQTT_MultipleTopics.ino** is an Arduino programm that is to be flashed on a ESp8266 chip. This will control the servos which will turn on/off lights
* **AlexaInteractionModel_MyDevices.json** is the model I used for my Alexa app in the Amazon Developer Console.
* I used **ngrok** to make my program live. 


## VARIABLES TO CHANGE: ##
Change the *MQTT_SERVER*, *ssid*, and *password* variables from **MQTT_MultipleTopics.ino**

## STEPS TO SETUP: ##
1) go to ngrok path and type: ./ngrok http 5000
2) add the https .io URL (example: https;//55055686.ngrok.io) as an endpoint in the Alexa Development Console 
3) in a seperate terminal, run the python script
4) upload the MQTT programm to the ESP8266

## ALEXA COMMANDS: ##

* "Alexa, tell my devices to turn {OnOff}" -> LightOnOffIntent
* "Alexa, tell my devices to dimmer {maxMinUpDown}" -> DimmerDirectionIntent
* "Alexa, tell my devices to turn dimmer {Angle} degrees -> DimmerAngleIntent

## mqtt PUBLISH AND SUBSCRIBE COMMANDS: ##
* `mosquitto_sub -v -t "/devices/light"`
* `mosquitto_pub -t "/devices/light" -m '1' //or '0'`
* `mosquitto_sub -v -t "/devices/dimmer"`
* `mosquitto_pub -t "/devices/dimmer" -m '__some_angle__'`

## FUTURE PLANS ##
Plan to re-write this application using Javascript and built a web app using Node, Express, and React

