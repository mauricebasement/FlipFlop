//Libraries
#include <EEPROM.h>
#include <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
#include <Time.h>         //http://www.arduino.cc/playground/Code/Time  
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include "TM1637.h"

//Pin Out
const int button1Pin = 10; //Button to switch between times
const int button2Pin = 11; //Up Button
const int button3Pin = 12; //Down Button
const int ledPin = 13; //Indicator LED
const int powerPin = 2; //Ballast Power Interrupt Signal
const int switchPin = 3; //Bulb Switching Signal

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
const int firstValue = 5-1; //-1 for double call in logic
const int secondValue = 19;
int ff = firstValue;
const int ffThreshold = 20;
const int secondThreshold = 30; 

//Variables for Switching function
boolean state = true;
boolean on = true;
const int delay1Switching = 300;
const int delay2Switching = 200;

//Time Variables with standart values
int switchHour = 11;
int switchMinute = 59;
int h=12;
int m=1;
int H;
int M;

//Point Indicator
boolean point = false;

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
    if(button2==HIGH && button2FFb == true) for(int i = 0; i < ff; i++) addMinute(); 
    if(button3==HIGH && button3FFb == true) for(int i = 0; i < ff; i++) subMinute();
    if(button2==HIGH && button2FF%2 == 1)addMinute();
    if(button3==HIGH && button3FF%2 == 1)subMinute();
  } else {
    displayTime();
    if(button2==HIGH && button2FFb == true) for(int i = 0; i < ff; i++) addM();
    if(button3==HIGH && button3FFb == true) for(int i = 0; i < ff; i++) subM();
    if(button2==HIGH && button2FF%2 == 1)addM();
    if(button3==HIGH && button3FF%2 == 1)subM();
  }
  
  checkSwitch();
  checkPoint();
  if(rtcCheck <= rtcCheckValue) {rtcCheck += 1; }
  else {
    readTimes();
    setRTC();
    rtcCheck = 0;
  }
  delay(100);
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
  if(on==true) switch1();
  switch2();
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
