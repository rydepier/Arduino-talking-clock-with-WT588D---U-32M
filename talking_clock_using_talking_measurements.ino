/*------------------------------------------------------------
Talking Clock with WT588D - U 32M audio module

Using audio file 'Talking Measurements'

Connections::
RTC and the OLED display are both I2C
they are connected in parallel

Vcc to Arduino 5 volts
Gnd to Arduino Gnd
SDA to Analog pin 4
SCL to Analog pin 5

(Be very careful with the OLED connections, they will burn out if not connected correctly)

WTD588D::
Upload the audio to this device before connecting into the circuit

Arduino pin 3 to Module pin 7 RESET
Arduino pin 4 to Module pin 21 BUSY
Arduino pin 5 to Module pin 18 PO1
Arduino pin 6 to Module pin 17 PO2
Arduino pin 7 to Module pin 16 PO3
Arduino Gnd to Module pin 14
Arduino 5v to Module pin 20

Push Button::
Connect N/O push switch between
Arduino pin 2
Arduino pin Gnd
(Note pin 2 is held HIGH by the internal pullup resistor)

By Chris Rouse
Feb 2016
------------------------------------------------------------*/
// incluide the Library
#include "WT588D.h"  // audio module
#include "U8glib.h"  // graphics
#include <SPI.h>
#include <Wire.h> // I2C
#include "RTClib.h"  // real time clock
#include <SoftwareSerial.h>
//
#define am 37
#define pm 113
#define oh 105
#define oclock 104
#define hourOffset 0
#define tensMinutesOffset 0x12
boolean andPhrase = false;
boolean minus = false; // true if number is less then 0
boolean halfSecond = true; //used to flash colon
int hundredsOffset = 0x1b; // 1 less than its actual value
int tensOffset = 0x12; // 2 less than its actual value
int pad = 0;
unsigned long previousMillis = 0; 
const long interval = 500;   
//
// setup u8g object
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C 
//
// Setup RTC
RTC_DS1307 RTC;
String thisTime = "";
boolean amFlag = true; // false if pm
int minuteValue;
int hourValue;
int b;
//
// set the correct pin connections for the WT588D chip
#define WT588D_RST 3  //Module pin "REST"
#define WT588D_CS 6   //Module pin "P02"
#define WT588D_SCL 7  //Module pin "P03"
#define WT588D_SDA 5  //Module pin "P01"
#define WT588D_BUSY 4 //Module pin "LED/BUSY" 
WT588D myWT588D(WT588D_RST, WT588D_CS, WT588D_SCL, WT588D_SDA, WT588D_BUSY);
#define talkPin 2 // pin used to request time
#define ledPin 13 // onboard LED
//
/*--------------------------------------------------------------*/
//
void setup() {
  Serial.begin(9600);
  pinMode(talkPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // turn off onboard LED
  // draw splash screen
  drawSplash();
  //  
  // initialize the chip and port mappiing
  myWT588D.begin();
  //
  // Setup RTC
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  // Get time
  DateTime now = RTC.now();
  hourValue = now.hour();
  minuteValue = now.minute();
  //    
  //
  // play boot up sound
  speakPhrase(116); // ready
  speakPhrase(60); // digital clock
}
/*--------------------------------------------------------*/
// main loop
/*--------------------------------------------------------*/

void loop() {
 // read the clock
  DateTime now = RTC.now();
  minuteValue = now.minute();
  // picture loop for clock
  u8g.firstPage();  
  do {
    drawClock();
  } while( u8g.nextPage() );
  //
 // see if the Talk Time button was pressed, this is attached to Arduino pin 2
  if(digitalRead(talkPin) == 0 ){ // talk key pressed
    speakTime();
  }
  // delay to flash colon
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) { 
    previousMillis = currentMillis; 
    halfSecond = !halfSecond;
  }
/*--------------------------------------------------------*/  
} // end main loop
/*--------------------------------------------------------*/ 
//
void drawClock(void) {
  // graphic commands to redraw the realtimeclock  
  u8g.setFont(u8g_font_profont15);
  u8g.drawStr(13,10, "Real Time Clock");
  if(amFlag){
    u8g.drawStr(110,15, "am"); 
  }
  else u8g.drawStr(110,38, "pm");
  u8g.setFont(u8g_font_profont29); 
  //
  //get current time
  DateTime now = RTC.now();
  // convert to 12 hour mode
  //
  // convert to 12 hour clock
  if(now.hour() >= 12){
    amFlag = false;
    hourValue = now.hour() - 12;
  }
  else amFlag = true; 
  if (now.hour() == 0) hourValue = 12; // midnight  
  // display time in 12 hour format
  thisTime="";
  if (halfSecond){
    thisTime=String(hourValue) + ":";
  }
  else thisTime=String(hourValue) + ".";
  if (now.minute() < 10){ thisTime=thisTime + "0";} // add leading zero if required
  thisTime=thisTime + String(now.minute());
  pad = thisTime.length(); // get the string length
  pad = 8 - pad; // max length - actual length
  pad = pad * 16; // number of pixels free
  pad = (pad/2) + 1; // find the start point to ensure string is centred
  const char* newTime = (const char*) thisTime.c_str();
  u8g.drawStr(pad,50, newTime);   
}
//
/*--------------------------------------------------------*/ 
//
void speakTime()
{
  DateTime now = RTC.now();  
  Serial.print("The time is: ");
  Serial.println(String(now.hour()) + ":" + String(now.minute()));
  //
  speakPhrase(0x88); // the time is
  // speak hour
  if(minuteValue == 0) speakPhrase(oclock);
  else speakPhrase(hourValue + hourOffset);
  //
  // speak minutes
  if(minuteValue > 0 && minuteValue < 21){
    if (minuteValue < 10){
      speakPhrase(oh);
    }
    speakPhrase(minuteValue);
  }
  if(minuteValue > 20){
    int temp = minuteValue/10;
    int temp2 = minuteValue;
    speakPhrase(temp + tensMinutesOffset);
    if((10* temp) != temp2){
      // only speak unit minutes if not 20, 30, 40 or 50
      speakPhrase(minuteValue - (10 * temp));
    }
  }
  // add am or pm
  if(amFlag){
    speakPhrase(am);
  }
  else speakPhrase(pm);
} // end speak time
/*--------------------------------------------------------*/
void busy(int pause){
  // waits for WT588D to finish sound
  delay(100);
  while (myWT588D.isBusy() ) {
  }
  delay(pause);
}

void speakPhrase(int phrase) {
  myWT588D.playSound(phrase);
  busy(0);
}
//
/*-----------------------------------------------------*/
void drawSplashScreen(){
    u8g.setFont(u8g_font_profont15); 
    u8g.drawStr(36,23, "Talking");
    u8g.drawStr(20,43, "Measurements");
    u8g.drawFrame(2,2,124,60);
    u8g.drawFrame(4,4,120,56);    
}

void drawSplash(){
      // picture loop for splash screen
    u8g.firstPage();  
    do {
      drawSplashScreen();
    } while( u8g.nextPage() );
}
/*--------------------------------------------------------*/
void speakNumber(int number){
  // will speak an integer from 1 to 9999
  // speak a number routine
  //
  if(number > 99){
   andPhrase = true;
  }
  else andPhrase = false;
  //
  if(minus) speakPhrase(0x5F); // minus
   // positive number over 1000
  if(number >= 1000){ 
    b = number/1000;
    speakPhrase(b);
    speakPhrase(0x8B); // thousand
    number = number - b*1000;
  }
  // positive number from 100 to 999
  if(number >=100){
    b = number/100;
    speakPhrase(b+ hundredsOffset);
    number = number - b*100;
    if(number > 0){
      speakPhrase(0x27); // short and
      andPhrase = false;
    }
  }
  // decade from 20 to 90
  if(number>=20){
    if(andPhrase ){
      speakPhrase(0x27); // short and
      andPhrase = false;
    }
    b = number/10;  
    speakPhrase(b + (tensOffset));
    number = number - b*10;
  }
  // below  20
  if(number>0){
    if(andPhrase){
      speakPhrase(0x27); // short and
    }
    speakPhrase(number);
  }
}
/*--------------------------------------------------------*/

