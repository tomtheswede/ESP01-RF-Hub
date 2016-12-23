// Import ESP8266 libraries
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* sensorID1 = "BUT008"; //Name of sensor
const char* sensorID2 = "BUT009"; //Name of sensor
const char* sensorID3 = "BUT010"; //Name of sensor
const char* deviceDescription1 = "RadioButton1";
const char* deviceDescription2 = "RadioButton2";
const char* deviceDescription3 = "RadioButton3";
const int readPin = 2;

//1 = 690 up then 310 down.
//0== 245 up then 775 down.
const long t1=200;
const long t2=600;
const long t3=800;
const long t4=1300;
int bitCount=0;
byte preamble=0;
int count=0;
long lastSendTime=millis();

unsigned long devID=0;
unsigned long msg=0;
boolean regBit=false;
unsigned long devIDs[5]={0,0,0,0,0};
unsigned long msgs[5]={0,0,0,0,0};
boolean regBits[5]={0,0,0,0,0};
unsigned int errors[5]={0,0,0,0,0};

unsigned long upTime=0;
unsigned long downTime=0;
unsigned int errorCount=0;
boolean record=false;
boolean nextBit=0;
boolean errorFlag=0;

bool riseFlag=false;
bool fallFlag=false;
long msRise=0;
long msFall=0;
long lastMessageTime=0;

//int indicatorPin = 14; //temp for debugging


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
  //pinMode(indicatorPin,OUTPUT);
  SendUdpValue("REG",sensorID1,String(deviceDescription1)); //Register LED on server
  SendUdpValue("REG",sensorID2,String(deviceDescription2)); //Register LED on server
  SendUdpValue("REG",sensorID3,String(deviceDescription3)); //Register LED on server
  
  attachInterrupt(digitalPinToInterrupt(readPin),changeInterrupt,CHANGE); //Comment out to remove button functionality
}

void loop() {
  //If a byte has finished sending, the riseFlag will fire
  if (riseFlag) {
    downTime=msRise-msFall;
    riseFlag=false;
  }
  if (fallFlag) {
    upTime=msFall-msRise;
    if (upTime>t1 && upTime<t2) {
      nextBit=0;
    }
    else if (upTime>t3 && upTime<t4) {
      nextBit=1;
    }
    else {
      errorFlag=true;
    }
    if (record) {
      if (errorFlag) {
        errorCount++;
        errorFlag=false;
      }
      else {
        if (bitCount<18) {
          bitWrite(devID,bitCount,nextBit);
        }
        else if (bitCount<18) {
          bitWrite(msg,bitCount,nextBit);
        }
        else if (downTime>1800) {
          checkOut();
          errorCount=errorCount+1000; //Special flag for bad preable read.
          record=false;
        }
        else {
          checkOut(); //Go to the ID and msg Checks
          record=false;
        }
      }
      bitCount++;
    }
    if (!record) {
      if (errorFlag) {
        preamble=0;
        errorFlag=false;
      }
      else {
        preamble=preamble<<1; //adds a zero to the byte
        preamble=preamble+nextBit;
        if (preamble==236) {
          regBit=true; //Register flag
          record=true;
          bitCount=0;
          preamble=0;
        }
        else if (preamble==125)  { // 01111101
          regBit=false; //regular preamble
          record=true;
          bitCount=0;
          preamble=0;
        }
      }
    }
  fallFlag=false;
  }
}

void changeInterrupt() { //What happens when the button pin changes value
  if (digitalRead(readPin)) {
    msRise=micros();
    riseFlag=true;
  }
  else {
    msFall=micros();
    fallFlag=true;
  }
}

void checkOut() {

  if (devID==167221 && (millis()-lastSendTime>800)) {  //  10101100101100010100000000000000
    SendUdpValue("LOG",sensorID1,"press");
    lastSendTime=millis();
  }
  if (devID==150837 && (millis()-lastSendTime>800)) {  //   01101001001100010000000000000000
    SendUdpValue("LOG",sensorID1,"longPress");
    lastSendTime=millis();
  }
  if (devID==35990 && (millis()-lastSendTime>800)) {  //  10101100101100010100000000000000
    SendUdpValue("LOG",sensorID2,"press");
    lastSendTime=millis();
  }
  if (devID==19606 && (millis()-lastSendTime>800)) {  //   01101001001100010000000000000000
    SendUdpValue("LOG",sensorID2,"longPress");
    lastSendTime=millis();
  }

/*
  Serial.print(regBit);
  Serial.print("\t");
  for (int i=0; i<32; i++) {
    Serial.print(bitRead(devID,i));
  }
  Serial.print("\t");
  for (int i=0; i<32; i++) {
    Serial.print(bitRead(msg,i));
  }
  Serial.print("\t");
  Serial.println(errorCount);
*/
  //delay(50); //To avoid double message counting due to RF signal redundancy
  devID=0;
  msg=0;
  errorCount=0;

  /*
  if (message=="1,1,3,1") {
    SendUdpValue("LOG",sensorID1,"press");
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
    delay(200);
  }
  else if (message=="p6932") {
    SendUdpValue("LOG",sensorID1,"longestPress");
    delay(200);
  }
  else if (message==",2,2e") {
    SendUdpValue("LOG",sensorID2,"press");
    delay(200);
  }
  else if (message=="65659") {
    SendUdpValue("LOG",sensorID2,"longestPress");
    delay(200);
  }
  */
  
  
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


