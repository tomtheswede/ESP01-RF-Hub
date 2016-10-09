// Import ESP8266 libraries
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* sensorID1 = "BUT007"; //Name of sensor
const char* deviceDescription = "TestButton";

byte buf[8] = {0,0,0,0,0,0,0,0};
int pingBuf[100];
long lastRead=0;
long preRead=0;
long ms1;
long ms2;
long ms3;
int readPin = 2;
int pingCount = 0;
int storeNum = 0;
String out[14] = {",","1","2","3","4","5","6","7","8","9","0",".","e","s"};
bool record=false;
bool checkOut=false;
String addString="";

// WiFi parameters
const char* ssid = ""; //Enter your WiFi network name here in the quotation marks
const char* password = ""; //Enter your WiFi pasword here in the quotation marks

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
  SendUdpValue("REG",sensorID1,String(deviceDescription)); //Register LED on server
  delay(2000); //Time clearance to ensure registration
  SendUdpValue("REG",sensorID1,String(deviceDescription)); //Register LED on server
}

void loop() {
  preRead=micros();
  if (micros()-lastRead>20) { //sets a minimum re-trigger time for the check of - this if statement and loop takes 3 microseconds
    // update buffer with 8 most recent values
    for (int i=0; i<7; i++) {
      buf[i]=buf[i+1];
    }
    buf[7]=digitalRead(readPin);
    lastRead=micros();
  }
  //600 & 500 work well,
  if (micros()-ms1>860 && buf[0]==0 && buf[1]==0 && buf[2]==0 && buf[3]==1 && buf[4]==1 && buf[5]==1 && buf[6]==1 && buf[7]==1) { //start timer for rise
    ms1=micros();
    pingCount++;
  }
  if (pingCount>14) {
    //Serial.println("pingCount Buffer exceeded");
    pingCount=0;
  }
  //1300 works well
  if (micros()-ms1>1600 && pingCount>0) { //between 270 and 400 works - 350 is good
    //Serial.println(pingCount); //This needs to be removed for correct timing
    if (pingCount==14 && not(record)) { //Start message
      //Serial.println("Start message"); //This needs to be removed for correct timing
      record=true;
    }
    if (record) {
      pingBuf[storeNum]=pingCount;
      //Serial.print("store"); //This needs to be removed for correct timing
      storeNum++;
      if (storeNum==20) {
        pingCount=13;
      }
    }
    if (pingCount==13 && record) { //end message and dump command
      //Serial.println("Stop record"); //This needs to be removed for correct timing
      record=false;
      checkOut=true;
    }
    pingCount=0;

  }
  
  if (micros()-ms1>30000) {
    ms1=micros();
  }

  if (checkOut && storeNum>2) {
    addString="";
    for (int i=0; i<storeNum; i++) {
      if (pingBuf[i]<=14) {
        addString=addString+out[pingBuf[i]-1];
      }
    }
    Serial.println(addString);
    checkOut=false;
    storeNum=0;
    if (addString=="s3,5e") {
      SendUdpValue("LOG",sensorID1,"toggle");
    }
  }
    
  
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


