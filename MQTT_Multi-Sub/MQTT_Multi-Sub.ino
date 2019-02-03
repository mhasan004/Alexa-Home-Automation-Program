//Raspberry pi + ESP8266: Programm to control a lamp and servo wirelessly using the MQTT Protocol and Alexa
// Subscribe to lamp: one on terminal type: mosquitto_sub -v -t "/test/lamp"
// Send lamp data to ESP8266 from Pi: mosquitto_pub -t "/test/lamp" -m '1' //or '0'
// Subscribe to servo: one on terminal type: mosquitto_sub -v -t "/test/servo"
// Send servo data to ESP8266 from Pi: mosquitto_pub -t "/test/servo" -m '__some_angle__'

#include <stdio.h>
#include <string.h>
#include <PubSubClient.h> 
#include <ESP8266WiFi.h>
#include <Servo.h>
Servo servo;

#define MQTT_SERVER "YOUR_LIUX_IP_ADDRESS"   //IP ADDRESS OF RASPBERRY PI MQTT SERVER IP. ifconfig to get it
const char* ssid = "YOUR_WIFI_NAME";         //NAME OF YOUR WIFI NETWORK
const char* password = "YOUR_WIFI_PASSWORD"; //PASSWORD OF WIFI
const int led_pin = 10;
const int lamp_pin = 4;                      //The LED is on ESP8266 GPIO
const int servo_pin = 5;
const char* topicsArry[] = {"/devices/lamp", "/devices/servo"}; //list of topics

//SETTING UP THE WIFI CLIENT AND THE PUBSUB CLIENT
void callback(char* topicChar, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, callback, wifiClient);

void setup() 
{
  Serial.begin(115200);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, LOW);
  pinMode(lamp_pin, OUTPUT);
  digitalWrite(lamp_pin, HIGH);
  servo.attach(servo_pin);
  servo.write(0);
  delay(1000);
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
  Serial.print("\n\nSubscribed to Topic:");
  Serial.print(topic);
  if (topic == "/devices/lamp")
  {
    /* CHANGE LIGHT BY CHECKING THE 1ST BYTE OF OUR AYLOAD. 
     * LIGHT ON if the payload[0] is '1' and publish to the MQTT server a confirmation message*/
    if(payload[0] == '1'){
      digitalWrite(lamp_pin, LOW);
      digitalWrite(led_pin, HIGH);
      client.publish("/devices/lamp", "Light On");
      Serial.println("Lamp ON");
    }
    /* LIGHT OFF if the payload is '0' and publish to the MQTT server a confirmation message*/
    else if (payload[0] == '0'){
      digitalWrite(lamp_pin, HIGH);
      digitalWrite(led_pin, LOW);
      client.publish("/devices/lamp", "Light Off");
      Serial.println("Lamp Off");
    }
  }
  else if (topic == "/devices/servo")
  {
    /*OVERVIEW: I tell Alexa to activate the intent 
     * --> Python code turns the number i said into a 3 digit string representation of my number and publishes it to the MQTT topic. 
     * --> The ESP8266 retrieves the payload and unpacks it (in this else if statement). 
     * --> It turns the each character bit (payload[0] to payload[2]) into its interger form <(char)payload[i]-48> and stores it into angleBit[]. 
     * --> Then it adds the bits to make the proper angle interger and stores it int angle.
     */
    int angleBit[3]={}; //payload is the ASCII numbers of the angle bit that was published. Will turn them back to integer
    for (int i=0; i<sizeof(payload)-1; ++i)      
      angleBit[i] = (char)payload[i]-48; //ex: payload[0] contains the ascii number for the character version of the angle. -48 that # to get the integer vlaue
    int angle = angleBit[0] * 100; 
    angle = angle + angleBit[1] * 10; 
    angle = angle + angleBit[2]; 
    Serial.println(angle);
    servo.write(angle);
    delay(500);
  }
}

 /*Generate unique name from MAC addr*/
 String macToStr(const uint8_t* mac){
   String result;
   for (int i = 0; i < 6; ++i) 
   {
     result += String(mac[i], 16);
     if (i < 5)
       result += ':';
   }
   return result;
 }
