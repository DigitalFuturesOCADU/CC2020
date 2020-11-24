/**
Experiment 5: Trading Temperatures.

****Storage and Playback must be enabled on your keys*****

2 channel setup
-publish on one channel
-read on a different channel

Inputs to update
-Name of your WIFI network
-Password of you WIFI network
-publish key
-subscribe key
-your ID name
-name of your data channel to publish on
-name of your partner's channel to read from

**/

#include <WiFiNINA.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>
#include <ArduinoJson.h>
#include <SparkFunLSM6DS3.h>
#include "Wire.h"

//**Details of your local Wifi Network

//Name of your access point
char ssid[] = "The name of your WIFI";
//Wifi password
char pass[] = "Your WIFI password";

int status = WL_IDLE_STATUS;       // the Wifi radio's status

// pubnub keys
char pubkey[] = "YOUR PUB KEY";
char subkey[] = "YOUR SUB KEY";

// channel and ID data

const char* myID = "Nick"; // place your name here, this will be put into your "sender" value for an outgoing messsage

char publishChannel[] = "nickData"; // channel to publish YOUR data
char readChannel[] = "kateData"; // channel to read THEIR data

// JSON variables
StaticJsonDocument<200> dataToSend; // The JSON from the outgoing message
StaticJsonDocument<200> inMessage; // JSON object for receiving the incoming values
//create the names of the parameters you will use in your message
String JsonParamName1 = "publisher";
String JsonParamName2 = "temperature";


int serverCheckRate = 1000; //how often to publish/read data on PN
unsigned long lastCheck; //time of last publish


//create variable for the IMU
LSM6DS3 myIMU(I2C_MODE, 0x6A); //Default constructor is I2C, addr 0x6B


//These are the variables that will hold the values we will be using
//some are calculated locally, some come from PubNub messages
int nickTemperature = 0;  
int kateTemperature = 0;  
float avgTemperature;
const char* inMessagePublisher; 






void setup() {
  
  Serial.begin(9600);

  //run this function to connect
  connectToPubNub();
  
  myIMU.begin();
}


void loop() 
{
//read temperature from IMU  
nickTemperature = myIMU.readTempC();

//send and receive messages with PubNub, based on a timer
sendReceiveMessages(serverCheckRate);

///Do whatever you want with the data here!

   
}

void connectToPubNub()
{
    // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to the network, SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    Serial.print("*");

    // wait 2 seconds for connection:
    delay(2000);
  }

  // once you are connected :
  Serial.println();
  Serial.print("You're connected to ");
  Serial.println(ssid);
  
  PubNub.begin(pubkey, subkey);
  Serial.println("Connected to PubNub Server");

  
}

void sendReceiveMessages(int pollingRate)
{
    //connect, publish new messages, and check for incoming
    if((millis()-lastCheck)>=pollingRate)
    {
      //publish data
      sendMessage(publishChannel); // publish this value to PubNub

      //check for new incoming messages
      readMessage(readChannel);
      
      //save the time so timer works
      lastCheck=millis();
    }


  
}



void sendMessage(char channel[]) 
{

      Serial.print("Sending Message to ");
      Serial.print(channel);
      Serial.println(" Channel");
  
  char msg[64]; // variable for the JSON to be serialized into for your outgoing message
  
  // assemble the JSON to publish
  dataToSend[JsonParamName1] = myID; // first key value is publisher
  dataToSend[JsonParamName2] = nickTemperature; // second key value is the temperature

  serializeJson(dataToSend, msg); // serialize JSON to send - sending is the JSON object, and it is serializing it to the char msg
  Serial.println(msg);
  
  WiFiClient* client = PubNub.publish(channel, msg); // publish the variable char 
  if (!client) 
  {
    Serial.println("publishing error"); // if there is an error print it out 
  }
  else
  {
  Serial.print("   ***SUCCESS"); 
  }

}

void readMessage(char channel[]) 
{
  String msg;
    auto inputClient = PubNub.history(channel,1);
    if (!inputClient) 
    {
        Serial.println("message error");
        delay(1000);
        return;
    }
    HistoryCracker getMessage(inputClient);
    while (!getMessage.finished()) 
    {
        getMessage.get(msg);
        //basic error check to make sure the message has content
        if (msg.length() > 0) 
        {
          Serial.print("**Received Message on ");
          Serial.print(channel);
          Serial.println(" Channel");
          Serial.println(msg);
          //parse the incoming text into the JSON object

          deserializeJson(inMessage, msg); // parse the  JSON value received

           //read the values from the message and store them in local variables 
           inMessagePublisher = inMessage[JsonParamName1]; // this is will be "their name"
           kateTemperature = inMessage[JsonParamName2]; // the value of their Temperature sensor

           //calculate the average of the 2 temperatures
           avgTemperature = (kateTemperature+nickTemperature)/2;

           Serial.print("Current Average KateNick Temp: ");
           Serial.println(avgTemperature);
        }
    }
    inputClient->stop();
  

}
