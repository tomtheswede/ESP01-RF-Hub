// Import ESP8266 libraries
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* sensorID1 = "BUT008"; //Name of sensor
const char* sensorID2 = "BUT009"; //Name of sensor
const char* sensorID3 = "BUT010"; //Name of sensor
const char* deviceDescription1 = "RadioButton1";
const char* deviceDescription2 = "RadioButton2";
const char* deviceDescription3 = "RadioButton3";

byte niblett[4];
int nibNum;
bool riseFlag=false;
bool fallFlag=false;
long msRise=0;
long msFall=0;
long fallLen=0;
long riseLen=0;
long startTime=0;
int readPin = 2;
int indicatorPin = 14; //temp for debugging
int readState=0;
int pingCount = 0;
int storeNum = 0;
String out[16] = {"0","1","2","3","4","5","6","7","8","9",".",",","e","p","f","s"};
bool record=false;
int highBits=0;
String addString="";

// WiFi parameters
const char* ssid = "TheSubway"; //Enter your WiFi network name here in the quotation marks
const char* password = "vanillamoon576"; //Enter your WiFi pasword here in the quotation marks

//Server details
unsigned int localPort = 5007;  //UDP send port
const char* ipAdd = "192.168.0.100"; //Server address
byte packetBuffer[512]; //buffer for incoming packets

WiFiUDP Udp; //Instance to send packets


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected with IP: ");
  // Print the IP address
  Serial.println(WiFi.localIP());

  //Open the UDP monitoring port
  Udp.begin(localPort);
  Serial.print("Udp server started at port: ");
  Serial.println(localPort);

  delay(2000); //Time clearance to ensure registration
  pinMode(indicatorPin,OUTPUT);
  SendUdpValue("REG",sensorID1,String(deviceDescription1)); //Register LED on server
  SendUdpValue("REG",sensorID2,String(deviceDescription2)); //Register LED on server
  SendUdpValue("REG",sensorID3,String(deviceDescription3)); //Register LED on server
  
  attachInterrupt(digitalPinToInterrupt(readPin),changeInterrupt,CHANGE); //Comment out to remove button functionality
}

void loop() {
  //If a byte has finished sending, the riseFlag will fire
  if (riseFlag && riseLen>200 && riseLen<700 && fallLen>700 && fallLen<1200) { //high low low - 0
    niblett[0]=niblett[1];
    niblett[1]=niblett[2];
    niblett[2]=niblett[3];
    niblett[3]=0;
    riseFlag=false;
    pingCount++;
  }
  else if (riseFlag && riseLen>700 && riseLen<1200 && fallLen>200 && fallLen<700) { //high high low - 1
    niblett[0]=niblett[1];
    niblett[1]=niblett[2];
    niblett[2]=niblett[3];
    niblett[3]=1;
    highBits=highBits+1; //for parity calculation
    riseFlag=false;
    pingCount++;
  }
  else if (riseFlag) { //If a signal has a bad bit, start again.
    riseFlag=false;
    pingCount=0;
    highBits=0;
    if (addString.length()>0) {
      checkOut(addString); //For debugging - detecting half-messages
    }
    addString="";
    record=false;
    digitalWrite(indicatorPin,LOW);
  }

  if (niblett[0]==0 && niblett[1]==1 && niblett[2]==1 && niblett[3]==1 && !record) {
    record=true; //Start recording
    digitalWrite(indicatorPin,HIGH);
    pingCount=0;
    highBits=0;
    startTime=millis();
  }
  else if (pingCount==4 && record) {
      //if ((highBits)%2) { //only even parity gets printed. Otherwise, in error
        addString=addString+out[nibNum];
        checkOut(addString);
      //}
      //else {
      //  Serial.println("Parity fail");
      //}
      addString="";
      record=false; //End recording
      digitalWrite(indicatorPin,LOW);
    }
    else if (millis()-startTime>200) {
      addString="";
      record=false;
      digitalWrite(indicatorPin,LOW);
    }
    else {
      nibNum=niblett[0]*8+niblett[1]*4+niblett[2]*2+niblett[3];
      //Serial.println(nibNum);
      addString=addString+out[nibNum];
    }
    pingCount=0;
  }
}

void changeInterrupt() { //What happens when the button pin changes value
  if (digitalRead(readPin)) {
    msRise=micros();
    fallLen=msRise-msFall;
    riseFlag=true;
  }
  else {
    msFall=micros();
    riseLen=msFall-msRise;
    fallFlag=true;
  }
  
}

void checkOut(String message) {
  Serial.println(message);
  if (message=="1,1,3,1") {
    SendUdpValue("LOG",sensorID1,"press");
  }
  else if (message=="1,1,3,2") {
    SendUdpValue("LOG",sensorID1,"longPress");
  }
  else if (message=="1,1,3,3") {
    SendUdpValue("LOG",sensorID1,"longerPress");
  }
  else if (message=="1,1,3,4") {
    SendUdpValue("LOG",sensorID1,"longestPress");
  }
  else if (message=="1,1,4,1") {
    SendUdpValue("LOG",sensorID2,"press");
  }
  else if (message=="1,1,4,2") {
    SendUdpValue("LOG",sensorID2,"longPress");
  }
  else if (message=="1,1,4,3") {
    SendUdpValue("LOG",sensorID2,"longerPress");
  }
  else if (message=="1,1,4,4") {
    SendUdpValue("LOG",sensorID2,"longestPress");
  }
  else if (message=="1,1,5,1") {
    //SendUdpValue("LOG",sensorID3,"press");
    Serial.println("Success");
  }
  else if (message=="1,1,5,2") {
    SendUdpValue("LOG",sensorID3,"longPress");
  }
  else if (message=="1,1,5,3") {
    SendUdpValue("LOG",sensorID3,"longerPress");
  }
  else if (message=="1,1,5,4") {
    SendUdpValue("LOG",sensorID3,"longestPress");
  }
  else if (message=="p6931") {
    SendUdpValue("LOG",sensorID1,"press");
    delay(100);
  }
  else if (message=="p6933") {
    SendUdpValue("LOG",sensorID2,"longestPress");
    delay(100);
  }
  //delay(50); //To avoid double message counting due to RF signal redundancy
}

void SendUdpValue(String type, String sensorID, String value) {
  //Print GPIO state in serial
  Serial.print("-Value sent via UDP: ");
  Serial.println(type + "," + sensorID + "," + value);

  // send a message, to the IP address and port
  Udp.beginPacket(ipAdd,localPort);
  Udp.print(type);
  Udp.write(",");
  Udp.print(sensorID);
  Udp.write(",");
  Udp.print(value); //This is the value to be sent
  Udp.endPacket();
}


