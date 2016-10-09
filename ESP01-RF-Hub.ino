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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  preRead=micros();
  if (micros()-lastRead>10) { //sets a minimum re-trigger time for the check of - this if statement and loop takes 3 microseconds
    // update buffer with 8 most recent values
    for (int i=0; i<7; i++) {
      buf[i]=buf[i+1];
    }
    buf[7]=digitalRead(readPin);
    lastRead=micros();
  }
  //600 & 500 work well,
  if (micros()-ms1>400 && buf[0]==0 && buf[1]==0 && buf[2]==0 && buf[3]==0 && buf[4]==1 && buf[5]==1 && buf[6]==1 && buf[7]==1) { //start timer for rise
    ms1=micros();
    pingCount++;
  }
  if (pingCount>14) {
    //Serial.println("pingCount Buffer exceeded");
    pingCount=0;
  }
  //1300 works well
  if (micros()-ms1>1300 && pingCount>0) { //between 270 and 400 works - 350 is good
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
    //ms3=micros();
    //Serial.print(ms3-ms2);
    //Serial.println(" micros for execution");
  }
  if (micros()-ms1>30000) {
    ms1=micros();
  }

  if (checkOut && storeNum>2) {
    for (int i=0; i<storeNum; i++) {
      if (pingBuf[i]<=14) {
        Serial.print(out[pingBuf[i]-1]);
      }
      if (pingBuf[i]==13) {
        Serial.println("");
      }
    }
    checkOut=false;
    storeNum=0;
  }
  //ms3=micros();
  //Serial.print(ms3-preRead);
  //Serial.println(" time for loop");

}


