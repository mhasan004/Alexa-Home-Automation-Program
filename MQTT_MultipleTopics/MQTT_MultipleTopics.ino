//WORKS - 2+ SUBSCRIPTION. This is code for ESP8266
//Raspberry pi + ESP8266: Simple programm to control a LED wirelessly using the MQTT Protocol and Alexa
/*SETUP ESP:
  * Need the Arduino Client for MQTT Protocol - PubSubClient: https://github.com/knolleary/pubsubclient
  * ESP8266WiFi Library: https://github.com/esp8266/Arduino
  * Arduino -> Preferences -> Additional Boards Manager URLS: copy this to the bar: new: https://arduino.esp8266.com/stable/package_esp8266com_index.json old: http://arduino.esp8266.com/versions/2.5.0-beta3/package_esp8266com_index.json
  * tools -> Board -> Board Manager -> scroll to the bottom and download esp8266 by ESP8266 community
  * tools -> boards -> NodeMCU 1.0 esp 12e
  * 
*/
// See what ESP Says: one on terminal type: mosquitto_sub -v -t "/devices/light"
// Send data to ESP from Pi: mosquitto_pub -t "/devices/light" -m '1' //or '0'

// See what ESP Says: one on terminal type: mosquitto_sub -v -t "/devicea/dimmer"
// Send data to ESP from Pi: mosquitto_pub -t "/devices/dimmer" -m '__some_angle__'

#include <stdio.h>
#include <string.h>
#include <PubSubClient.h> 
#include <ESP8266WiFi.h>
#include <Servo.h>
Servo dimmer;

#define MQTT_SERVER "IP ADDRESS OF SERVER/RASPBERRY PI"         // IP ADDRESS OF RASPBERRY PI MQTT SERVER IP. ifconfig to get it
const char* ssid = "YOUR WIFI ADDRESS";            // NAME OF YOUR WIFI NETWORK
const char* password = "WIFI PASWORD"; // PASSWORD OF WIFI
const int led_pin = 10;                     // SD3
const int lightRelay_pin = 4;               // D2 - The LED is on ESP8266 GPIO 4 - D2
const int dimmer_pin = 5;                   // D1
const char* topicsArry[] = {"/devices/light", "/devices/dimmer"}; //list of topics

//SETTING UP THE WIFI CLIENT AND THE PUBSUB CLIENT
void callback(char* topicChar, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() 
{
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  pinMode(lightRelay_pin, OUTPUT);
  digitalWrite(lightRelay_pin, HIGH);
  /*START THE WIFI SUB-SYSTEM:*/
  WiFi.begin(ssid, password);
  reconnect();  //this function tries to connect to the WIFI network, then it connects to the MQTT server
  delay(2000);  //wait 2 secs before startign the main program
}

void loop()
{
  /*Reconnect to the WIFI or the MQTT server if the connection is lost*/
  if (!client.connected() && WiFi.status() == 3)
    reconnect();
  client.loop();  //RUN THE MQTT LOOP to maintain the connection and look for any new messages to run the callback fucntion. 
  delay(10);   //need to delay to allwo ESP8266 WIFI functions to run
}

/*CONNECT TO MQTT NETWORK AND SUBSCRIBE TO TOPICS*/
void reconnect() // Loop until we're reconnected
{
  while (!client.connected() && WiFi.status() == WL_CONNECTED) 
  {
    /*1)Generate client name based on MAC address*/
    Serial.print("\nAttempting MQTT connection...");
    String clientName;
    clientName += "esp8266-";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);
    /*2) SUBSCRIBE TO TOPIC*/
    if (client.connect((char*) clientName.c_str()))    // Attempt to connect
    {
      Serial.println("Connected to MQTT Server");
      for (int i=0; i<sizeof(topicsArry)/sizeof(char *); ++i)
      {
        client.subscribe(topicsArry[i]);
        Serial.print("Subscribed to "); 
        Serial.println(topicsArry[i]); 
      }
    } 
    else{
      Serial.println("\nFailed."); 
      reconnect();
    }
  }
}


/* WHEN THE TOPICS YOU ARE SUBSCRIBED TO HAS AN UPDATE, READ IT*/
/* This function will be called whenever there is a new message from our topic. */
void callback(char* topicChar, byte* payload, unsigned int length) 
{
  String topic = topicChar; //apprantly topicChar isnt a topic so i cant compare it with == for the if statements. So need to equal it to a string
  Serial.print("\n\nSubscribed to Topic: ");
  Serial.print(topic);
 // Serial.print(topic);
  Serial.print("       lightTopic?: ");
  Serial.print(topic == "/devices/light");
  Serial.print("   dimmerTopic?: ");
  Serial.print(topic == "/devices/dimmer");
  if (topic == "/devices/light")
  {
    /* CHANGE LIGHT BY CHECKING THE 1ST BYTE OF OUR AYLOAD. 
     * LIGHT ON if the payload[0] is '0' and publish to the MQTT server a confirmation message*/
    if(payload[0] == '0')
    {
      digitalWrite(lightRelay_pin, HIGH);
      digitalWrite(led_pin, HIGH);
      client.publish("/devices/light", "Light On");
      Serial.println("    Light ON");
    }
    /* LIGHT OFF if the payload is '1' and publish to the MQTT server a confirmation message*/
    else if (payload[0] == '1')
    {
      digitalWrite(lightRelay_pin, LOW);
      digitalWrite(led_pin, LOW);
      client.publish("/devices/light", "Light Off");
      Serial.println("     Light Off");
    }
  }
  else if (topic == "/devices/dimmer")
  {
    /*OVERVIEW: I tell Alexa to activate the intent 
     * --> Python code turns the number i said into a 3 digit string representation of my number and publishes it to the MQTT topic. 
     * --> The ESP8266 retrieves the payload and unpacks it (in this else if statement). 
     * --> It turns the each character bit (payload[0] to payload[2]) into its interger form <(char)payload[i]-48> and stores it into angleBit[]. 
     * --> Then it adds the bits to make the proper angle interger and stores it in angle.
     */
    int angleBit[3]={}; //payload is the ASCII numbers of the angle bit that was published. Will turn them back to integer
    for (int i=0; i<sizeof(payload)-1; ++i)      
      angleBit[i] = (char)payload[i]-48; //ex: payload[0] contains the ascii number for the character version of the angle. -48 that # to get the integer vlaue
    int angle = angleBit[0] * 100; 
    angle = angle + angleBit[1] * 10; 
    angle = angle + angleBit[2]; 
    Serial.print("\nTURNING SERVO TO ANGLE: ");
    Serial.print(angle);
    dimmer.attach(dimmer_pin);
    dimmer.write(angle);
    delay(3000);
    dimmer.detach();
  }
}

 /*Generate unique name from MAC addr*/
 String macToStr(const uint8_t* mac){
   String result;
   for (int i = 0; i < 6; ++i) 
   {
     result += String(mac[i], 16);
     if (i < 5){
       result += ':';
     }
   }
   return result;
 }
