# Alexa, tell my lamp to turn {OnOff} -> LampOnOffIntent 
# Alexa, tell my servo to turn {LeftRight} -> ServoDirectionIntent
# Alexa, tell my servo to turn {Angle} degrees -> ServoAngleIntent

from flask import Flask, template_rendered
from flask_ask import Ask, statement, question
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
    return question("I didnt get that. You can say lamp on or off. Or you can say Servo, and then say an angle value")

@ask.intent('AMAZON.CancelIntent')
@ask.intent('AMAZON.StopIntent')
def stop():
    return statement("Ok Bye")

@ask.intent('Amazon.HelpIntent')
def help():
    return question("You can say tell lamp to turn on, or, turn servo 90 degrees")

#LAMP ON/OFF:
@ask.intent('LampOnOffIntent')
def lamp(OnOff):
    topic = "/devices/lamp"
    if OnOff is None: #no command was given
        return question("Do you want to turn LAMP On or off?")
    elif OnOff == "on" or OnOff == "off":
        if OnOff == "on": #turn ON lamp
            print("on")
            os.system("""mosquitto_pub -t %s -m '1'""" %topic)
        else: #turn OFF lamp
            print("off")
            os.system("""mosquitto_pub -t %s -m '0'""" %topic)
        return statement( "Turning lamp %s" %OnOff) 
    # else: #a valid command was not given
    #     return question("do you want to turn your lamp on, or , off")

#SERVO

@ask.intent('ServoAngleIntent')
def servoAngle(Angle):
    if Angle is None: #no command was given
        return question("Tell me what angle to turn the servo")
    else:
        topic = "/devices/servo"
        print("Setting Servo to angle: %s \n" %Angle)
        formatted_Angle = '%03d' % int(Angle) #turned Angle number to a 3 digit string. SOLUTION to issue where the ESP8266 gets some random numbers passed
        os.system("""mosquitto_pub -t %s -m %s""" %(topic, formatted_Angle) )
        return statement("Turning Servo to Angle: %s" %formatted_Angle)

@ask.intent('ServoDirectionIntent')
def servoDirection(LeftRight):
    servoAngle(LeftRight)

if __name__ == '__main__':
    app.run(debug=True)



