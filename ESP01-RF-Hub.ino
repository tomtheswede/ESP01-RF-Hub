// Import ESP8266 libraries
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* sensorID1 = "BUT008"; //Name of sensor
const char* deviceDescription = "TestButton2";

byte niblett[4];
int nibNum;
bool riseFlag=false;
bool fallFlag=false;
long msRise=0;
long msFall=0;
long fallLen=0;
long riseLen=0;
long startTime=0;
int readPin = 14;
int readState=0;
int pingCount = 0;
int storeNum = 0;
String out[16] = {"0","1","2","3","4","5","6","7","8","9",".",",","e","p","f","s"};
bool record=false;
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
  SendUdpValue("REG",sensorID1,String(deviceDescription)); //Register LED on server
  //delay(2000); //Time clearance to ensure registration
  //SendUdpValue("REG",sensorID1,String(deviceDescription)); //Register LED on server
  
  attachInterrupt(digitalPinToInterrupt(readPin),changeInterrupt,CHANGE); //Comment out to remove button functionality
}

void loop() {
  if ((riseFlag || fallFlag) && (riseLen<150 || fallLen<150 || riseLen>2000 || fallLen>2000)) { //Noise filter
    riseFlag=false;
    fallFlag=false;
  }
  else if (riseFlag) {
    riseFlag=false;
    fallFlag=false;
    if (riseLen>=150 && riseLen<500 && fallLen>=500 && fallLen<900) {
      niblett[0]=niblett[1];
      niblett[1]=niblett[2];
      niblett[2]=niblett[3];
      niblett[3]=0;
      pingCount++;
    }
    else if (riseLen>=500 && riseLen<900 && fallLen>=150 && fallLen<500) {
      niblett[0]=niblett[1];
      niblett[1]=niblett[2];
      niblett[2]=niblett[3];
      niblett[3]=1;
      pingCount++;
    }
    else {
      pingCount=0;
    }
  }

  if (niblett[0]==1 && niblett[1]==1 && niblett[2]==1 && niblett[3]==1 && !record) {
    record=true;
    pingCount=0;
    startTime=millis();
  }
  else if (pingCount==4 && record) {
    if (niblett[0]==1 && niblett[1]==1 && niblett[2]==0 && niblett[3]==1 && addString.length()>0) { //end recording
      checkOut(addString);
      addString="";
      record=false;
    }
    else if (millis()-startTime>200) {
      addString="";
      record=false;
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
  if (message=="4,9") {
    SendUdpValue("LOG",sensorID1,"press");
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


