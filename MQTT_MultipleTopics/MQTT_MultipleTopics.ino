//WORKS - 2+ SUBSCRIPTION. This is code for ESP8266
//Raspberry pi + ESP8266: Simple programm to control a LED wirelessly using the MQTT Protocol and Alexa
/*SETUP THE ESP8266 LIBRARIES:
  * Need the Arduino Client for MQTT Protocol - PubSubClient: https://github.com/knolleary/pubsubclient OR 8266.com/stable/package_esp8266com_index.json old: http://arduino.esp8266.com/versions/2.5.0-beta3/package_esp8266com_index.json
  * ESP8266WiFi Library: https://github.com/esp8266/Arduino
  * Arduino -> Preferences -> Additional Boards Manager URLS: copy this to the bar: new: https://arduino.esp
  * tools -> Board -> Board Manager -> scroll to the bottom and download esp8266 by ESP8266 community
  * tools -> boards -> NodeMCU 1.0 esp 12e*/
// PROCESS: you tell Alexa a command, alexa activates the python decorator function that is linked to the Alexa intent. This function publishes something to the topics. The ESP is subscribed to those topics and recieves the information and turn lights on. 
/* SUBSCRIPE ESP TO LIGHT TOPIC: mosquitto_sub -v -t "/devices/light" 
 * Send data to ESP from Pi: mosquitto_pub -t "/devices/light" -m '1' //or '0'
 * SUBSCRIPE ESP TO DIMMER TOPIC: mosquitto_sub -v -t "/devicea/dimmer"
 * Send data to ESP from Pi: mosquitto_pub -t "/devices/dimmer" -m '__some_angle__'  */

/*************************SETUP*******************************************************************/
#include <stdio.h>
#include <string.h>
#include <Servo.h>
Servo dimmer;
const int led_pin = 10;           // SD3
const int dimmer_DataPin = 5;     // D1
const int dimmer_MosfetPin = 4;   // D2
int offCounter = 0; //how many loops counter is on
bool switchit = true;
/*************************WIFI SETUP*******************************************************************/
#include <Arduino.h>       //conserve power section
#include <PubSubClient.h>  //MQTT
#include <ESP8266WiFi.h>   //WIFI
extern "C" {               //conserve power section
    #include "user_interface.h"  // Required for wifi_station_connect() to work
}
#define FPM_SLEEP_MAX_TIME 0xFFFFFFF //conserve power section
void WiFiOn();
void WiFiOff();
const char* ssid = "YOUR WIFI ADDRESS";            // NAME OF YOUR WIFI NETWORK
const char* password = "WIFI PASSWORD"; // PASSWORD OF WIFI
WiFiClient wifiClient;
/*************************MQTT SETUP*******************************************************************/
#define MQTT_SERVER "IP ADDRESS OF YOUR RASPBERRY PI"         // IP ADDRESS OF RASPBERRY PI MQTT SERVER IP. ifconfig to get it
void callback(char* topicChar, byte* payload, unsigned int length);
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient); //(ipAddressOfBroker, Broker Port, function to start upon update, wifi to connect to)
const char* topics[] = {"/devices/light", "/devices/dimmer"}; //list of topics

void mqttPublish(char topic[], char message[]){
  client.publish(topic, message);
}
void mqttPublish(char topic[], int num){
  char c[16];
  itoa(num, c, 10);
  client.publish(topic, c);
}
void wifiConnect(){
  Serial.print("\nConnecting to wifi");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("   CONNECTED");
}
void WiFiOn() {
    wifi_fpm_do_wakeup();
    wifi_fpm_close();
    Serial.println("WifiOn");
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();
    delay(1000);
}
void WiFiOff() {
    Serial.println("WifiOff");
    client.disconnect();
    wifi_station_disconnect();
    wifi_set_opmode(NULL_MODE);
    wifi_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);   
    delay(1000);
}


void setup() 
{
  Serial.begin(38400);
  pinMode(led_pin, OUTPUT);
  pinMode(dimmer_MosfetPin, OUTPUT);  
  digitalWrite(led_pin, LOW);
  digitalWrite(dimmer_MosfetPin, LOW);
  /*CONNECT TO WIFI AND MQTT SERVERM:*/
  wifiConnect();
  reconnect();  //this function tries to connect to the WIFI network, then it connects to the MQTT server
}

void loop()
{ 
  if (!client.connected() && WiFi.status() == 3){reconnect();}
  client.loop();  //RUN THE MQTT LOOP to maintain the connection and look for any new messages to run the callback fucntion. 
  delay(10);   //need to delay to allwo ESP8266 WIFI functions to run 


  
}

/*CONNECT TO MQTT NETWORK AND SUBSCRIBE TO TOPICS*/
void reconnect() // Loop until we're reconnected
{
  while (!client.connected() && WiFi.status() == WL_CONNECTED ) //WL_CONNECTED = 3 = wifi is connected
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
      for (int i=0; i<sizeof(topics)/sizeof(char *); ++i)
      {
        client.subscribe(topics[i]);
        Serial.print("Subscribed to "); 
        Serial.println(topics[i]); 
      }
    } 
    else{
      Serial.println("\nFailed."); 
      reconnect();
    }
  }

}



/* WHEN THE TOPICS YOU ARE SUBSCRIBED TO HAS AN UPDATE, READ IT
   ***Whenever i tell Alexa to do something, Alexa will activate the decorator function on my Pi, which will publish something to a topic. in the void loop, client.loop() will activate this call back function
   Basically: This function will be called whenever there is a new message from our topic. */
void callback(char* topicChar, byte* payload, unsigned int length) 
{
  Serial.print("\n.....IN CALLBACK.....");Serial.print("\nSubscribed to Topic: ");Serial.print(String(topicChar));
  String topicStr = topicChar;
  Serial.print("   pay: ");
  Serial.print(payload[0]);
  Serial.print("   topic: ");
  Serial.print(topicStr);
  Serial.print("   ");

  /*SUBSCRIPTION HANDLING*/
  if (topicStr == "/devices/light" )//topicChar isnt a string so will make it a ctring to compare with == 
  {
    /* CHANGE LIGHT BY CHECKING THE 1ST BYTE OF OUR PAYLOAD. 
     * LIGHT ON if the payload[0] is '1' and publish to the MQTT server a confirmation message*/
    if(payload[0] == '1')
    {
      digitalWrite(led_pin, HIGH);
      Serial.println("    Light ON");
      mqttPublish("/feedback/light", "light On");
    }
    /* LIGHT OFF if the payload is '0' and publish to the MQTT server a confirmation message*/
    else if (payload[0] == '0')
    {
      digitalWrite(led_pin, LOW);
      Serial.println("     Light Off");
      mqttPublish("/feedback/light", "light Off");
    }
  }
  else if (topicStr == "/update/dimmer")
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

    digitalWrite(dimmer_MosfetPin, HIGH);//giveing power to the base of the transistor so that the emitter and the collectors are connected (servo grounded to power supply)
    dimmer.attach(dimmer_DataPin); //attaching the dimmer servo so that i can feed it angles
    dimmer.write(angle);    
    Serial.print("\nTURNING SERVO TO ANGLE: "); Serial.print(angle); Serial.print("       start_dimmerPowerPinD2: "); Serial.println(digitalRead(dimmer_MosfetPin));
    delay(1000);
    digitalWrite(dimmer_MosfetPin, LOW); 
    dimmer.detach();
    mqttPublish("/feedback/dimmer", angle);
  }
}

/*Generate unique name from MAC addr*/
String macToStr(const uint8_t* mac){
 String result;
 for (int i = 0; i < 6; ++i) {
   result += String(mac[i], 16);
   if (i < 5){
     result += ':';
   }
 }
 Serial.print("\nmacresult: ");
 Serial.println(result); 
 return result;
}
