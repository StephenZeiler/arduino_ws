#include <Arduino.h>
#include <Stepper.h>

const int stepPinM1 = 11; //11 is pin location 
const int dirPinM1 = 10;
const int enPinM1 = 8;
const int stepPinM2 = 5; //5 is pin location 
const int dirPinM2 = 6;
const int enPinM2 = 9;
const int s1Pin = 0;
const int s2Pin = 1;
boolean s1 = false;
boolean s2 = false;
int val = 0;
void setBool(){
  s1=true;
}
void setBool2(){
  s2 = true;
}
void runMotorM1() {
digitalWrite(dirPinM2,HIGH);
    for(int x=0; x<1; x++){

    digitalWrite(stepPinM1,HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPinM1,LOW);
    delayMicroseconds(500);
    }
}
void runMotorM2() {
digitalWrite(dirPinM2,HIGH);
    for(int x=0; x<1; x++){

    digitalWrite(stepPinM2,HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPinM2,LOW);
    delayMicroseconds(500);
    }
}

void setup() {
  pinMode(stepPinM1, OUTPUT);
  pinMode(dirPinM1, OUTPUT);
  pinMode(enPinM1, OUTPUT);
  pinMode(stepPinM2, OUTPUT);
  pinMode(dirPinM2, OUTPUT);
  pinMode(enPinM2, OUTPUT);
  pinMode(s1Pin, INPUT);
  pinMode(s2Pin, INPUT);
    digitalWrite(enPinM1, LOW);
  digitalWrite(enPinM2, LOW);

}


void loop() {
 // attachInterrupt(digitalPinToInterrupt(s1Pin), setBool, FALLING);
if(analogRead(s1Pin)==LOW){
  s1 = false;
 while(s1 == false){
  runMotorM1();
  runMotorM2();
  if(analogRead(s2Pin) == LOW){

  s1 = true;
  }
  
}
  

  }
}