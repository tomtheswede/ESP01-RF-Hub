byte buf[10] = {0,0,0,0,0,0,0,0,0,0};
int pingBuf[100];
long ms1;
long ms2;
int readPin = 4;
int pingCount = 0;
int pingNum = 0;
String out[14] = {",","1","2","3","4","5","6","7","8","9","0",".","e","s"};
bool record=false;
bool checkOut=false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i<9; i++) {
    buf[i]=buf[i+1];
  }
  buf[9]=digitalRead(readPin);
  if (buf[0]==1 && buf[1]==1 && buf[2]==0 && buf[3]==0) {
    ms1=micros();
  }
  if (buf[0]==0 && buf[1]==0 && buf[2]==1 && buf[3]==1) {
    ms2=micros();
    pingCount++;
  }
  
  if (ms2-ms1>320) { //between 270 and 400 works - 350 is good
    //Serial.println(pingCount);
    if (pingCount==14 && not(record)) { //Start message
      record=true;
      pingNum=0;
    }
    if (record) {
      pingBuf[pingNum]=pingCount;
      pingNum++;
    }
    if (pingCount==13 && record) { //end message and dump command
      record=false;
      checkOut=true;
      clearBuf();
    }
    ms1=ms2;
    pingCount=0;
  }

  if (checkOut) {
    for (int i=0; i<pingNum; i++) {
      Serial.print(out[pingBuf[i]-1]);
      if (pingBuf[i]==13) {
        Serial.println("");
      }
    }
    checkOut=false;
  }
  delayMicroseconds(10);
}

void clearBuf() {
  for (int i=0; i<10; i++) {
    buf[i]=0;
  }
}
