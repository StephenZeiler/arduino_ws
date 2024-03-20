#include <Arduino.h>
#include <Stepper.h>
//NOTES:
//If room is needed for additional digital ports, move a INPUT to a analog port to free up space. 
//Caution using digital port 0. It reads high automatically and can prevent the arduino from working. 
//Sensors can be used on Analog and Digital ports..
//Analog ports can only read INPUTS!

//Speaker
const int startBuzzer = 15;

// Counter
const int counter = 13;

//LED 
const int homeButtonLED = 4;
const int startButtonLED = 12;

//Ram
const int ramPin = 19;

//Air Release
const int airBlastPin = 20;

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
//Sensors can be used on Analog and Digital ports..
//Analog ports can only read INPUTS!
const int s1Pin = A0; //Home position sensor
const int s2aPin = 17; //M2 Home position sensor
const int s2bPin = A1; //M2 Away position sensor
const int s3aPin = 1; //M3 Home position sensor 
const int s3bPin = A5; //M3 Away position sensor
const int s4Pin =  14; //Pipet feed (full/empty)
const int s5Pin =  A9; //Cap feed (full/empty)
const int s6Pin =  16; //Exit sensor
const int s7Pin =  18; //Overun sensor
const int s8Pin = 21; //Pipet tube sensor

//Buttons
const int stepperButtonPin = A7;
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
int LEDSwitch = 1;
unsigned long previousSlowStepM1 = 0;
unsigned long previousM1Micros = 0;  
unsigned long previousM2Micros = 0;  
unsigned long previousM3Micros = 0; 
long previousHomeLEDMicros = 0;  
long m1Speed = 1000; // 1000 is 70/min.... 750 = 90/min... If change, to change speed change the m1 speed in else of runMotorM1()...
long m2Speed = 380; //150 is 70/min.... 95 = 90/min @800 steps/rev --- 380 is 90/min @200 steps/rev
long m3Speed = 520; //200 is 70/min.... 130 = 90/min @800 steps/rev --- 520 is 90/min @200 steps/rev
double m1PulsePerRevMultiplier = 0.9; //.9 for 400, .45 for 800 on driver
bool ejectionFailed = false;
bool ejectionDetected = false;
int encoderCurrentState;
int encoderPreviousState;
int encoderCount = 0;
bool emptyCaps = false;
bool empytOverunCaps = false;
bool emptyPipets = false;
bool stopPressed = false;
bool resetPositions = false;

long calculateDegrees(long rotaryPosition) //converts the steps the stepper has stepped to degrees //a 400 step goes 0.9 degrees per step. 200 stepper motor is 1.8 degrees per step. Currently 800!
{
  long result = rotaryPosition * m1PulsePerRevMultiplier; 
  return result;
}
void activateStartBuzzer(){
  tone(startBuzzer, 1000);
   // digitalWrite(startBuzzer, HIGH);
  delay(2000);
  noTone(startBuzzer);
  //digitalWrite(startBuzzer, LOW);
}
bool checkOverunCaps(){
  if(digitalRead(s7Pin) == LOW){
    empytOverunCaps = true;
  }
  else{
    empytOverunCaps = false;
  }
}
bool checkLoadedPipet(){
  if(digitalRead(s4Pin) == LOW){
    emptyPipets = true;
  }
  else{
    emptyPipets = false;
  }
}
bool checkLoadedCaps(){
  if(digitalRead(s5Pin) == LOW){
    emptyCaps = true;
  }
  else{
    emptyCaps = false;
  }
}
bool preCheckCond()
{
  bool preCheckReady = false;
  bool s2aReady = false;
  bool s3aReady = false;
  bool s4Ready = false;
  bool s5Ready = false;
  bool s8Ready = false;
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
    emptyPipets = false;
  }
  if (digitalRead(s5Pin) == HIGH)
  {
    s5Ready = true;
    emptyCaps = false;
  }
  if (digitalRead(s8Pin) == LOW)
  {
    s8Ready = true;
  }
  if (s2aReady && s3aReady && s4Ready && s5Ready && s8Ready)
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
void blinkButtonLED(int pinLED)
{
  unsigned long currentMicros = micros();
  for (int x = 0; x < 1; x++)
  {
    if((currentMicros - previousHomeLEDMicros)> 200000 )
    {
      if (LEDSwitch == 1)
      {
        digitalWrite(pinLED, HIGH);
        ++LEDSwitch;
      }
      else if (LEDSwitch == 2)
      {
        digitalWrite(pinLED, LOW);
        LEDSwitch = 1;
      }
      previousHomeLEDMicros = currentMicros;
    }

  }
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
  {
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
  if(digitalRead(stopButtonPin)==HIGH || ejectionFailed || empytOverunCaps || emptyPipets || emptyCaps){
    stopPressed = true;
  }
  if(digitalRead(s6Pin) == HIGH){
    digitalWrite(counter, HIGH);
    ejectionDetected = true;
  }
  else{
    digitalWrite(counter, LOW);
  }
  if(calculateDegrees(rotaryPosition)==345 && ejectionDetected == false){
    ejectionFailed = true;
  }
    digitalWrite(dirPinM1, HIGH);
  unsigned long currentMicros = micros();
  for (int x = 0; x < 1; x++)
  {
    if (analogRead(s1Pin) == LOW && slowStart == false) //check when motor has made full revolution
    {
      checkLoadedPipet();
      checkLoadedCaps();
      rotaryPosition = 0; // made full circle reset position
      ejectionDetected = false;
      if(stopPressed){
        slowStart = true;
        readyToStart = false;
        digitalWrite(ramPin, LOW);
        productionRun = false;
      }
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
void stepM1()
{
  digitalWrite(dirPinM1, HIGH);
  digitalWrite(stepPinM1, HIGH);
  delayMicroseconds(9000);
  digitalWrite(stepPinM1, LOW); 
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
void initializeM2ToHomePos()
{
  digitalWrite(dirPinM2, HIGH);
  bool atHome = false;
  while (atHome == false)
  {
    if (digitalRead(s2aPin) == HIGH)
    {
      atHome = true;
    }
    else
    {
      runMotorM2();
    }
  }
}
void initializeM3ToHomePos()
{
  bool atHome = false;
  while (atHome == false)
  {
    if (digitalRead(s3aPin) == HIGH)
    {
      atHome = true;
    }
    else
    {
      runMotorM3();
    }
  }
}
void setup()
{
  //Speaker
  pinMode(startBuzzer, OUTPUT);
  //digitalWrite(startBuzzer, LOW);
 // noTone(startBuzzer);

  //Counter
  digitalWrite(counter, LOW);

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
  pinMode(counter, OUTPUT);
  pinMode(s1Pin, INPUT);
  pinMode(s2aPin, INPUT);
  pinMode(s2bPin, INPUT);
  pinMode(s3aPin, INPUT);
  pinMode(s3bPin, INPUT);
  pinMode(s4Pin, INPUT);
  pinMode(s5Pin, INPUT);
  pinMode(s6Pin, INPUT);
  
  //Buttons
  pinMode(stepperButtonPin, OUTPUT);
  pinMode(homeButtonPin, OUTPUT);
  pinMode(startButtonPin, OUTPUT);
  pinMode(stopButtonPin, OUTPUT);
}

void loop()
{
  checkOverunCaps();
  int stepperButtonState = digitalRead(stepperButtonPin);
  int homeButtonState = digitalRead(homeButtonPin);
  int startButtonState = digitalRead(startButtonPin);
  unsigned long currentMicros = micros();
  if(stepperButtonState == HIGH && !productionRun){
    activateStartBuzzer();
    while(digitalRead(stepperButtonPin) == HIGH){
      stepM1();
      readyToStart = false;
    }
  }
  if(!readyToStart){
    blinkButtonLED(homeButtonLED);
  }
  else{
    digitalWrite(homeButtonLED,LOW);
  }
  if(readyToStart && !productionRun){
    blinkButtonLED(startButtonLED);
  }
  else if(productionRun){
    digitalWrite(startButtonLED,LOW);
  }
  if(homeButtonState==HIGH && !readyToStart){
    ejectionFailed = false;
    readyToStart = true;
    activateStartBuzzer();
    initializeM1ToHomePos();
    initializeM2ToHomePos();
    initializeM3ToHomePos();
    digitalWrite(ramPin, LOW);
  }
  if(startButtonState == HIGH && readyToStart){
    if(preCheckCond()){
      productionRun = true;
      stopPressed = false;
      activateStartBuzzer();
    }
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
      if(calculateDegrees(rotaryPosition)  == 165 && !empytOverunCaps){
        digitalWrite(ramPin, HIGH);
      }
      if(calculateDegrees(rotaryPosition)  == 280){
        digitalWrite(ramPin, LOW);
        digitalWrite(airBlastPin, HIGH);
      }
      if(calculateDegrees(rotaryPosition) == 330){
        digitalWrite(airBlastPin, LOW);
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
      if(calculateDegrees(rotaryPosition) > 45 && calculateDegrees(rotaryPosition) < 186 && m3IsHome==true){
        runMotorM3();
      }
      if(calculateDegrees(rotaryPosition) > 186 && m3IsHome==false){
        runMotorM3();
      }
    }
  }
}