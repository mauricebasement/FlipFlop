//Libraries
#include <EEPROM.h>
#include <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
#include <Time.h>         //http://www.arduino.cc/playground/Code/Time  
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include <TM1637.h>

#include "pinOut.h"

//RTC Check Counter
float rtcCheck = 0;
const float rtcCheckValue=100000;

//Variables to store button states
int  button1;
int  button2;
int  button3;

//Variables for Fast Forward function
int button2FF = 0;
int button3FF = 0;
boolean button2FFb;
boolean button3FFb;
const int firstValue = 2;
const int secondValue = 20;
int ff = firstValue;
const int ffThreshold = 30;
const int secondThreshold = 45; 

//Variables for Switching function
boolean state = true;
boolean on = true;
const int delay1Switching = 1000; //Between interuption of power and bulb switching
const int delay2Switching = 1000; //Between bulb switching and end of powerinteruption

//Time Variables with standart values
int switchHour = 11;
int switchMinute = 59;
int h=12;
int m=1;
int H;
int M;

//Point Indicator
boolean point = false;

//Main Delay
const int mainDelay = 100;

//Digit Display
#define CLK 8   
#define DIO 9
TM1637 tm1637(CLK,DIO);

void setup()
{
  readTimes(); //Read Times from RTC Memory
  tm1637.init(); //Init 4 Digit LED Display
  tm1637.point(POINT_ON);    
  tm1637.set(BRIGHT_DARKEST);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  setRTC();
  pinMode(ledPin,OUTPUT);
  pinMode(switchPin,OUTPUT);
  pinMode(powerPin,OUTPUT);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);
  pinMode(button3Pin, INPUT);
  if (checkClock(hour(),minute()) == false) {
    digitalWrite(powerPin,HIGH);
    digitalWrite(ledPin,HIGH);
    digitalWrite(switchPin,HIGH);
    state = false;
  } else {
    digitalWrite(powerPin,HIGH);
    digitalWrite(ledPin,LOW);
    digitalWrite(switchPin,LOW);
    state = true;
  }
}

void loop()
{
  button1 = digitalRead(button1Pin);   
  button2 = digitalRead(button2Pin); 
  button3 = digitalRead(button3Pin);
  if(button2 == HIGH) { button2FF += 1; } else { button2FF = 0; }
  if(button3 == HIGH) { button3FF += 1; } else { button3FF = 0; }
  if (button2FF >= ffThreshold) { button2FFb = true;} else { button2FFb = false; }
  if (button3FF >= ffThreshold) { button3FFb = true; } else { button3FFb = false; }
  ff = firstValue;
  if (button2FF >= secondThreshold) ff=secondValue;
  if (button3FF >= secondThreshold) ff=secondValue;
  if(button1== HIGH) {
    displaySwitchTime(); 
    if(button2==HIGH) {
      if(button2FFb == true) { for(int i = 0; i < ff; i++) addMinute(); 
      } else { addMinute(); }
    }
    if(button3==HIGH) {
      if(button3FFb == true) { for(int i = 0; i < ff; i++) subMinute();
      } else { subMinute(); }
    }
  } else {
    displayTime();
    if(button2==HIGH) {
      if(button2FFb == true) { for(int i = 0; i < ff; i++) addM();
      } else { addM(); }
    }
    if(button3==HIGH)
      if(button3FFb == true) { for(int i = 0; i < ff; i++) subM(); 
      } else { subM(); }
  }
  checkSwitch();
  checkPoint();
  if(rtcCheck <= rtcCheckValue) {rtcCheck += 1; }
  else {
    readTimes();
    setRTC();
    rtcCheck = 0;
  }
}
void readTimes() {
  tmElements_t tmp;
  RTC.read(tmp);
  h=tmp.Hour;
  m=tmp.Minute;
  switchMinute = tmp.Year -30;
  switchHour = tmp.Month-1;  
}
void checkPoint() {
  if(point==false) {
    tm1637.point(POINT_ON);
    point=true;
  } else {
    tm1637.point(POINT_OFF);
    point=false;
  }
}
void displayTime() {
  tm1637.display(0,hour() / 10);
  tm1637.display(1,hour() % 10); 
  tm1637.display(2,minute() / 10);
  tm1637.display(3,minute() % 10);
}
void displaySwitchTime() {
  tm1637.display(0,switchHour / 10);
  tm1637.display(1,switchHour % 10); 
  tm1637.display(2,switchMinute / 10);
  tm1637.display(3,switchMinute % 10);
}
//Set Switch Time
void addMinute() {
  if(switchMinute==59) {
    switchMinute=0;
    addHour();
  } else {
    switchMinute+=1;
    setRTC();
  }
}
void subMinute() {
  if(switchMinute==0) {
    switchMinute=59;
    subHour();
  } else {
    switchMinute-=1;
    setRTC();
  }  
}
void addHour() {
  if(switchHour==11) {
    switchHour=0;
  } else {
    switchHour+=1;
  }
  setRTC();
}
void subHour() {
  if(switchHour==0) {
    switchHour=11;
  } else {
    switchHour-=1;
  }  
  setRTC();
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
  H = switchHour + 1;
  M = 2000 + switchMinute;
  setTime(h, m, 1, 1, H, M);
  //setTime(h, m, 1, 1, 1, 1);
  RTC.set(now());          
}
//AddHours Helper Method
int addHours(int input, int increment) {
  int sum = input + increment;
  if (sum >= 24) return sum -= 24;
  return sum;
}
//Switching Methods
void checkSwitch() {  
  on = checkClock(hour(),minute());
  if(on==true) { switch1(); 
  } else {  switch2(); }
  delay(mainDelay);
  digitalWrite(powerPin, LOW);
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
void switch1() {
  if (state != on) {
  digitalWrite(powerPin, HIGH);
  delay(delay1Switching);
  digitalWrite(switchPin, LOW);
  digitalWrite(ledPin, LOW);
  delay(delay2Switching);
  state = true;
  digitalWrite(powerPin, LOW);
  }
} 
void switch2() { 
  if (state != on) {
  digitalWrite(powerPin, HIGH);
  delay(delay1Switching);
  digitalWrite(switchPin, HIGH);
  digitalWrite(ledPin, HIGH);
  delay(delay2Switching);
  state = false;
  digitalWrite(powerPin, LOW);
  }
}
