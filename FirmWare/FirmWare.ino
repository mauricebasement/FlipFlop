#include <EEPROM.h>
#include <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
#include <Time.h>         //http://www.arduino.cc/playground/Code/Time  
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include "TM1637.h"

//Pin Out
const int button1Pin = 10;
const int button2Pin = 11;
const int button3Pin = 12;
const int ledPin =4;
const int powerPin = 2;
const int switchPin =13;

int  button1;
int  button2;
int  button3;

boolean state = true;
boolean on = true;

//Switch Time
int switchHour = 11;
int switchMinute = 59;

//Set Time
int h = 12;
int m = 1;

//Digit Display
#define CLK 8   
#define DIO 9
TM1637 tm1637(CLK,DIO);


void setup()
{
    Serial.begin(9600);
    setSyncProvider(RTC.get);
    if(timeStatus() != timeSet) Serial.println("Unable to sync with the RTC");
    else Serial.println("RTC has set the system time");
    tm1637.init();
    tm1637.point(POINT_ON);    
    tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
    setRTC();
    pinMode(ledPin,OUTPUT);
    pinMode(switchPin,OUTPUT);
    pinMode(powerPin,OUTPUT);
    
    pinMode(button1Pin, INPUT);
    pinMode(button2Pin, INPUT);
    pinMode(button3Pin, INPUT);
}

void loop()
{
  button1 = digitalRead(button1Pin);   
  button2 = digitalRead(button2Pin); 
  button3 = digitalRead(button3Pin);
  
  if(button1== HIGH) {
    displaySwitchTime(); 
    if(button2==HIGH)addMinute();
    if(button3==HIGH)subMinute();
  } else {
    displayTime();
    if(button2==HIGH)addM();
    if(button3==HIGH)subM();
  }
  checkSwitch();
  
  delay(10);
}

void displayTime()
{
  tm1637.display(0,hour() / 10);
  tm1637.display(1,hour() % 10); 
  tm1637.display(2,minute() / 10);
  tm1637.display(3,minute() % 10);
}

void displaySwitchTime()
{
  tm1637.display(0,switchHour / 10);
  tm1637.display(1,switchHour % 10); 
  tm1637.display(2,switchMinute / 10);
  tm1637.display(3,switchMinute % 10);
}
//Set Switch Time
void addMinute() {
  if(switchMinute==59) {
    addHour();
    switchMinute=0;
  } else {
    switchMinute+=1;
  }
}
void subMinute() {
  if(switchMinute==0) {
    subHour();
    switchMinute=59;
  } else {
    switchMinute-=1;
  }  
}
void addHour() {
  if(switchHour==11) {
    switchHour=0;
  } else {
    switchHour+=1;
  }
}
void subHour() {
  if(switchHour==0) {
    switchHour=11;
  } else {
    switchHour-=1;
  }  
}
//Set Time
void addM() {
  if(m==59) {
    m=0;
    addH();
  } else {
    m+=1;
    setRTC();
  }
}
void subM() {
  if(m==0) {
    m=59;
    subH();
  } else {
    m-=1;
    setRTC();
  }  
}
void addH() {
  if(h==23) {
    h=0;
  } else {
    h+=1;
  }
  setRTC();
}
void subH() {
  if(h==0) {
    h=23;
  } else {
    h-=1;
  }
  setRTC();  
}
void setRTC() {
    time_t t;
    tmElements_t tm;
    tm.Year = 30;
    tm.Month = 1;
    tm.Day = 1;
    tm.Hour = h;
    tm.Minute = m;
    tm.Second = 0;
    t = makeTime(tm);
    RTC.set(t);
   
}
void checkSwitch() {  
  tmElements_t tim;
  RTC.read(tim);
  h=tim.Hour;
  m=tim.Minute;
  on = checkClock(h,m);
  if(on==true) { switch1();
  }else{ switch2(); }
}
boolean checkClock(int hh, int mm) {
    if (hh == switchHour) {
      if(mm <= switchMinute) return false;
      return true; 
    }
    if(hh == addHours(switchHour,12 )) {
      if(mm <= switchMinute) return true;
      return false;
    }
    if(hh >= switchHour && hh < addHours(switchHour,12)) {
      return true;
    }
  return false;
}

int addHours(int input, int increment) {
  int sum = input + increment;
  if (sum >= 24) return sum -= 24;
  return sum;
}
void switch1() {
  if (state != on) {
  digitalWrite(powerPin, HIGH);
  delay(1000);
  digitalWrite(switchPin, LOW);
  delay(1000);
  state = true;
  digitalWrite(powerPin, LOW);
  }
}
  
void switch2() { 
  if (state != on) {
  digitalWrite(powerPin, HIGH);
  delay(1000);
  digitalWrite(switchPin, HIGH);
  delay(1000);
  state = false;
  digitalWrite(powerPin, LOW);
  }
}
