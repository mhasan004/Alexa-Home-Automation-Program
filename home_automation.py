# Alexa, tell my lamp to turn {OnOff} -> LampOnOffIntent 
# Alexa, tell my dimmer to turn {LeftRight} -> dimmerDirectionIntent
# Alexa, tell my dimmer to turn {Angle} degrees -> dimmerAngleIntent

# See what ESP Says: one on terminal type: mosquitto_sub -v -t "/test/light"
# Send data to ESP from Pi: mosquitto_pub -t "/test/light" -m '1' //or '0'
# See what ESP Says: one on terminal type: mosquitto_sub -v -t "/test/dimmer"
# Send data to ESP from Pi: mosquitto_pub -t "/test/dimmer" -m '__some_angle__'
from flask import Flask, template_rendered
from flask_ask import Ask, statement, question
from numpy import interp #trying to map percent 0-100 to servo angle 0-180
import subprocess
import os
import sys #to write on terminal

app = Flask(__name__)
ask = Ask(app, '/')

@app.route('/')
def index():
    return "will put buttons here to control devices"

@ask.launch
def launch():
    return question("This is the Device Alexa skill. tell me what to do")

@ask.intent('AMAZON.FallbackIntent')
def fallback():
    return question("I didnt get that. You can say light on or off. Or you can say Dimmer of or off or give a percentage")

@ask.intent('AMAZON.CancelIntent')
@ask.intent('AMAZON.StopIntent')
def stop():
    return statement("Ok Bye")

@ask.intent('AMAZON.HelpIntent')
def help():
    return question("You can say tell lamp to turn on, or, turn dimmer 90 degrees")

#LAMP ON/OFF:
@ask.intent('LightOnOffIntent')
def lamp(OnOff):
    topic = "/devices/light"
    if OnOff is None: #no command was given
        return question("Do you want to turn the light On or off?")
    elif OnOff == "on" or OnOff == "off":
        os.system("""mosquitto_sub -v -t "/feedback/light""")
        if OnOff == "on": #turn ON lamp
            os.system("""mosquitto_pub -t %s -m '1'""" %topic)
        
        else: #turn OFF lamp
            os.system("""mosquitto_pub -t %s -m '0'""" %topic)
        
        return statement( "Turning light %s" %OnOff) 
    # else: #a valid command was not given
    #    return question("do you want to turn your lamp on, or , off")

#Dimmer Servo
@ask.intent('DimmerAngleIntent')
def DimmerAngle(Angle):
    if Angle is None: #no command was given
        return question("What percentage should the dimmer be?")
    else:
        #NOTES: So When i tell Alexa a Angle number, it activates this function and passes in the Angle
        #BUT i need to cast Angle to an int before i map it. FOr some reason it dont work if i dont do this
        topic = "/devices/dimmer"
        Angle = int(interp(int(Angle),[0,100],[180,0])) #mapping percent to angle an dmaking it an int from a float
        print("\nSetting dimmer to percent: %s \n" %Angle)
        formattedPercent = '%03d' % int(Angle) #turned Angle number to a 3 digit string. SOLUTION to issue where the ESP8266 gets some random numbers passed
        os.system("""mosquitto_pub -t %s -m %s""" %(topic, formattedPercent) )
        return statement("OK")

@ask.intent('DimmerDirectionIntent')
def DimmerDirection(MaxMinUpDown):
    if MaxMinUpDown is None: #no command was given
        return question("Tell me what direction to turn the dimmer, up or down")
    else:
        topic = "/devices/dimmer"
        Angle = 90;
        if (MaxMinUpDown == "up") or (MaxMinUpDown == "max") or (MaxMinUpDown == "on"):
            Angle = 0;            
            print("\nSetting dimmer to max\n")
        if (MaxMinUpDown == "down") or (MaxMinUpDown == "min") or (MaxMinUpDown == "off"):
            Angle = 180;            
            print("\nSetting dimmer to low\n")
        formattedPercent  = '%03d' % int(Angle) #turned Angle number to a 3 digit string. SOLUTION to issue where the ESP8266 gets some random numbers passed
        os.system("""mosquitto_pub -t %s -m %s""" %(topic, formattedPercent) )
        return statement("OK")

if __name__ == '__main__':
    app.run(port=8000, debug=True)



