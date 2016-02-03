/*------------------------------------------------------------
 * Talking Clock with WT588D - U 32M audio module
 */
// incluide the Librarys
#include "WT588D.h"
#include <Wire.h>
#include "RTClib.h"
//
// setup RTC
RTC_DS1307 RTC;
// set the correct pin connections for the WT588D chip
#define WT588D_RST 7  //Module pin "REST"
#define WT588D_CS 6   //Module pin "P02"
#define WT588D_SCL 9  //Module pin "P03"
#define WT588D_SDA 8  //Module pin "P01"
#define WT588D_BUSY 5 //Module pin "LED/BUSY" 

#define talkPin 2 // pin used to request time
#define ledPin 13 // onboard LED

WT588D myWT588D(WT588D_RST, WT588D_CS, WT588D_SCL, WT588D_SDA, WT588D_BUSY);
//
// Name the various words and phrases
#define _am 0
#define _pm 44
#define timeIs 1
#define boot 2


//
// Variables
int timeString[3];
boolean am = true; // if false then pm
String clockTime;
int lastTime = 61;
//


void setup() {
  Serial.begin(9600);
  // set RTC
  Wire.begin();
  pinMode(talkPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  // initialize the chip and port mappiing
  myWT588D.begin();
  //
  am = true;
  // play boot up sound
  myWT588D.playSound(45);
  busy(200); // wait for WT588D to finish sound 

}

void loop(){
  DateTime now = RTC.now();
  if(now.hour() <13){
    timeString[0] = now.hour();
    am = true;
  }
  else{
    timeString[0] = now.hour()- 12;
    am = false; 
  }
  if(timeString[0] == 0){ // for a 12 hour clock there is no 0 hour
    timeString[0] = 12;
  }
  timeString[1] = now.minute();
  // see if the Talk Time button was pressed, this is attached to Arduino pin 2
  if(digitalRead(talkPin) == 0){ 
    speakTime();
   } 
  // un rem this to show time on Serial Monitor
  /*if(lastTime != timeString[1]){
    Serial.print(timeString[0]);
    Serial.print(":");
    if(timeString[1] < 10){Serial.print("0");}
    Serial.println(timeString[1]);
    lastTime = timeString[1];
  }
   */
}

void busy(int pause){
  // waits for WT588D to finish sound
  delay(100);
  while (myWT588D.isBusy() ) {
  }
  delay(pause);
}

void speakTime() {
  myWT588D.playSound(timeIs);
  busy(10);
  // speak hour
  myWT588D.playSound(timeString[0]+2);
  busy(200);  
  // is it the top of the hour?
  if (timeString[1] == 0){
    myWT588D.playSound(15); 
    busy(200);  
  }
  else{
    // speak mins from 1 to 20
    if(timeString[1] < 21){
      if(timeString[1] <10){
        myWT588D.playSound(16);
        busy(200);
      }
      myWT588D.playSound(timeString[1] + 20);
      busy(200); 
    }
    // speak minutes 21 to 59
    if (timeString[1] > 20){
      myWT588D.playSound((timeString[1]/10) + 15);
      busy(10);
      int a = (timeString[1] - (10*(timeString[1]/10)));
      if(10*a > 0){
        myWT588D.playSound(a + 20);
        busy(200); 
      }          
    }
  }
  //play either am or pm
  if(am){
    myWT588D.playSound(_am);
    busy(200);
  }
  else{
    myWT588D.playSound(_pm); 
    busy(200);  
  }
}
