#include <Arduino.h>
#include <Stepper.h>

//Ram
const int ramPin = 19;

//Air Release
const int airBlastPin = 20;

//Encoder
const int s0CLKPin = 12;
const int s0DTPin = 13;

//Motors
const int stepPinM1 = 11; //PUL+ Green
const int dirPinM1 = 10; //DIR+ Blue
const int enPinM1 = 9; //ENA+ Red

const int stepPinM2 = 6; //PUL+ Green
const int dirPinM2 = 5; //DIR+ Blue
const int enPinM2 = 8; //ENA+ Red

const int stepPinM3 = 3; //PUL+ Green
const int dirPinM3 = 7; //DIR+ Blue
const int enPinM3 = 2; //ENA+ Red

//Sensors

const int s1Pin = A0;
const int s2aPin = 17; //change to digital "red ports"
const int s2bPin = A1;
const int s3aPin = 1; //change to digital "red ports"
const int s3bPin = A5; 
const int s4Pin =  14; //change to digital "red ports"
const int s5Pin =  15; //change to digital "red ports"
const int s6Pin =  16; //change to digital "red ports"
const int s7Pin =  18; //change to digital "red ports"

//Buttons
const int homeButtonPin = A14;
const int startButtonPin = A11;
const int stopButtonPin = A10;


bool productionRunM2 = false;
bool m2IsHome = true;
bool m3IsHome = true;
bool productionRunM3 = false;
bool slowStart = true;
bool readyToStart = false;
bool productionRun = false;
volatile boolean turnDetected;
long rotaryPosition;
long previousPosition;
int stepsToTake;
bool s1 = false;
bool s2 = false;
bool preCheckPass = false;
int val = 0;
int m1Step = 1;
int m2Step = 1;
int m3Step = 1;
unsigned long previousM1Micros = 0;  
unsigned long previousM2Micros = 0;  
long previousM3Micros = 0;  
long m1Speed = 1000; // 1000 If change, must change slow start speed as well...
long m2Speed = 150; //200
long m3Speed = 200; //250
double m1PulsePerRevMultiplier = 0.9; //.9 for 400, .45 for 800 on driver
bool ejectionFailed = false;
bool ejectionDetected = false;
int encoderCurrentState;
int encoderPreviousState;
int encoderCount = 0;

long calculateDegrees(long rotaryPosition) //converts the steps the stepper has stepped to degrees //a 400 step goes 0.9 degrees per step. 200 stepper motor is 1.8 degrees per step. Currently 800!
{
  long result = rotaryPosition * m1PulsePerRevMultiplier; 
  return result;
}
void initializeM1ToHomePos()
{
  digitalWrite(dirPinM1, HIGH);
  bool atHome = false;
  while (atHome == false)
  {
    if (analogRead(s1Pin) == LOW)
    {
      atHome = true;
    }
    else
    {
      digitalWrite(stepPinM1, HIGH);
      delayMicroseconds(9000);
      digitalWrite(stepPinM1, LOW); 
      previousPosition = rotaryPosition;
      rotaryPosition = rotaryPosition + 1;
    }
  }
  rotaryPosition = 0; // set position to 0.
}

bool preCheckCond()
{
  bool preCheckReady = false;
  bool s2aReady = false;
  bool s3aReady = false;
  bool s4Ready = false;
  bool s5Ready = false;
  if (digitalRead(s2aPin) == HIGH)
  {
    s2aReady = true;
  }
  if (digitalRead(s3aPin) == HIGH)
  {
    s3aReady = true;
  }
  if (digitalRead(s4Pin) == HIGH)
  { 
    s4Ready = true;
  }
  if (digitalRead(s5Pin) == HIGH)
  {
    s5Ready = true;
  }
  if (s2aReady && s3aReady && s4Ready && s5Ready)
  {
    preCheckReady = true;
  }
  return preCheckReady;
}
bool ejectionCheck()
{       
  bool temp = false;
  if(calculateDegrees(rotaryPosition)>359 && ejectionDetected == false){
    temp = true;
  }
  return temp; // if temp is true then ejection failed. 
}

void runMotorM3()
{
    unsigned long currentMicros = micros();
  digitalWrite(dirPinM3, LOW);
  for (int x = 0; x < 1; x++)
  {
    if((currentMicros - previousM3Micros)> m3Speed)
    {
      if (m3Step == 1)
      {
        digitalWrite(stepPinM3, HIGH);
        ++m3Step;
      }
      else if (m3Step == 2)
      {
        digitalWrite(stepPinM3, LOW);
        m3Step = 1;
      }
      previousM3Micros = currentMicros;
    }
  }
}
void runMotorM2()
{
    unsigned long currentMicros = micros();
  digitalWrite(dirPinM2, HIGH);
  for (int x = 0; x < 1; x++)
  {
if((currentMicros - previousM2Micros)> m2Speed)
  { // Moved down here where it belongs: Got ya.
    if(m2Step ==1){
    digitalWrite(stepPinM2, HIGH);
      ++m2Step;
    }
    else if(m2Step ==2){
      digitalWrite(stepPinM2, LOW);
      m2Step = 1;
    }
  previousM2Micros = currentMicros; 
  
  }
  }
}
void runMotorM1()
{
  if(digitalRead(s6Pin) == HIGH){
    ejectionDetected = true;
  }
  if(calculateDegrees(rotaryPosition)==345 && ejectionDetected == false){
    ejectionFailed = true;
  }
    digitalWrite(dirPinM1, HIGH);
  unsigned long currentMicros = micros();
  for (int x = 0; x < 1; x++)
  {
    if (analogRead(s1Pin) == LOW && slowStart == false)
    {
      rotaryPosition = 0; // made full circle reset position
      ejectionDetected = false;
    }
    if(slowStart && rotaryPosition * m1PulsePerRevMultiplier < 10){
      m1Speed = 3000;
    }
     else if(slowStart && rotaryPosition * m1PulsePerRevMultiplier < 15){
      m1Speed = 2000;
    }
    else if(slowStart && rotaryPosition * m1PulsePerRevMultiplier < 20){
      m1Speed = 1400;
    }
    else{
      slowStart = false;
      m1Speed = 1000; 
    }

    if((currentMicros - previousM1Micros)> m1Speed){
      if(m1Step ==1){
        digitalWrite(stepPinM1, HIGH);
        ++m1Step;
        previousPosition = rotaryPosition;
        rotaryPosition = rotaryPosition + 1;
      }
      else if(m1Step ==2){
          digitalWrite(stepPinM1, LOW);
          m1Step = 1;
      }
      previousM1Micros = currentMicros; 
    }
  }
}
void setup()
{
  
  //Air release
  pinMode(airBlastPin, OUTPUT);

  //Ram
  pinMode(ramPin, OUTPUT);

  //Motors
  
  pinMode(stepPinM1, OUTPUT);
  pinMode(dirPinM1, OUTPUT);
  pinMode(enPinM1, OUTPUT);
  pinMode(stepPinM2, OUTPUT);
  pinMode(dirPinM2, OUTPUT);
  pinMode(enPinM2, OUTPUT);
  pinMode(stepPinM3, OUTPUT);
  pinMode(dirPinM3, OUTPUT);
  pinMode(enPinM3, OUTPUT);
  digitalWrite(enPinM1, LOW);
  digitalWrite(enPinM2, LOW);
  digitalWrite(enPinM3, LOW);

  //Sensors
  //pinMode(s0DTPin, INPUT);
  pinMode(s1Pin, INPUT);
  pinMode(s2aPin, INPUT);
  pinMode(s2bPin, INPUT);
  pinMode(s3aPin, INPUT);
  pinMode(s3bPin, INPUT);
  pinMode(s4Pin, INPUT);
  pinMode(s5Pin, INPUT);
  pinMode(s6Pin, INPUT);
  
  //Buttons
  pinMode(homeButtonPin, OUTPUT);
  pinMode(startButtonPin, OUTPUT);
  pinMode(stopButtonPin, OUTPUT);

}

void loop()
{
  // int temp = digitalRead(s6Pin);
  // if(calculateDegrees(rotaryPosition)>10 && temp == HIGH){
  //   ejectionDetected = true;
  // }
  int homeButtonState = digitalRead(homeButtonPin);
  int startButtonState = digitalRead(startButtonPin);
  int stopButtonState = digitalRead(stopButtonPin);
  unsigned long currentMicros = micros();
  if(homeButtonState==HIGH && !readyToStart){
    if(preCheckCond()){
      ejectionFailed = false;
      readyToStart = true;
      initializeM1ToHomePos();
    }
  }
  if(startButtonState == HIGH && readyToStart){
    productionRun = true;
  }

 // if(stopButtonState==HIGH || ejectionCheck()){
  if(stopButtonState==HIGH || ejectionFailed){
    slowStart = true;
    productionRun = false;
    readyToStart = false;
    //ejectionDetected = false;
   // rotaryPosition = 0;
  }

  if (productionRun)
  {
    runMotorM1();
    if(!slowStart){
      if(digitalRead(s2aPin)==HIGH){
        m2IsHome = true;
      }
      if(digitalRead(s2bPin)==LOW){
        m2IsHome = false;
      }
      if(calculateDegrees(rotaryPosition)  == 165){
        digitalWrite(ramPin, HIGH);
      }
      if(calculateDegrees(rotaryPosition)  == 280){
        digitalWrite(ramPin, LOW);
      }
      if(calculateDegrees(rotaryPosition) < 185 && m2IsHome==true){
        runMotorM2();
      }
      if(calculateDegrees(rotaryPosition) > 185 && m2IsHome==false){
        runMotorM2();
      }
      if(digitalRead(s3aPin)==HIGH){
        m3IsHome = true;
      }
      if(digitalRead(s3bPin)==LOW){
        m3IsHome = false;
      }
      if(calculateDegrees(rotaryPosition) < 186 && m3IsHome==true){
        runMotorM3();
      }
      if(calculateDegrees(rotaryPosition) > 186 && m3IsHome==false){
        runMotorM3();
      }
    }
  }

}