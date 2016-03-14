#include <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
#include <Time.h>         //http://www.arduino.cc/playground/Code/Time  
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire (included with Arduino IDE)
#include "TM1637.h"

#define CLK 8   
#define DIO 9

const int button1Pin = 10;     // the number of the pushbutton pin
const int button2Pin = 11;     // the number of the pushbutton pin
const int button3Pin = 12;     // the number of the pushbutton pin

int  button1;
int  button2;
int  button3;

int switchHour = 12;
int switchMinute = 0;

TM1637 tm1637(CLK,DIO);

void setup(void)
{
    Serial.begin(9600);
    setSyncProvider(RTC.get);   // the function to get the time from the RTC
    if(timeStatus() != timeSet) 
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");
        
    tm1637.init();
    tm1637.point(POINT_ON);    
    tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;  
}

void loop(void)
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
  }
  delay(100);
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
  if(switchHour==23) {
    switchHour=0;
  } else {
    switchHour+=1;
  }
}
void subHour() {
  if(switchHour==0) {
    switchHour=23;
  } else {
    switchHour-=1;
  }  
}
