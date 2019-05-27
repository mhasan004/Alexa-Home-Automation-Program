# Devices_Alexa_Program
This custom program to turn on/off a lamp and turn a servo a certain angle using Alexa, Raspberry Pi, and the ESP8266 through the MQTT protocol 

**home_automation.py** is the python script that used Flask and Flask-Ask libraries to process Alexa intents. It will send servo angle values to the ESP8266 by publishing them to the MQTT server.

**MQTT_MultipleTopics.ino** is an Arduino program that is to be flashed on a ESp8266 chip. This will controll the servos which will turn on/off lights

**AlexaInteractionModel_MyDevices.json** is the model I used for my Alexa app in the Amazon Developer Console.

I used **ngrok** to take my program live. 

