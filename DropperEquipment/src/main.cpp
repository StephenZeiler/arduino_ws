#include <Arduino.h>
#include <Stepper.h>

int b1State = 0; //Pre-check button 
int b2State = 0; //production button
int b3State = 0; // Stop button
bool readyToStart = false;
bool productionRun = false;
volatile boolean turnDetected;
int rotaryPosition;
int previousPosition;
int stepsToTake;

const int decoderCLK = 3;
const int decoderDT = 2;

const int ledS2a = ? ; // digital
const int ledS3a = ? ; // digital

const int stepPinM1 = 11; // 11 is pin location
const int dirPinM1 = 10;
const int enPinM1 = 8;
const int stepPinM2 = 5; // 5 is pin location
const int dirPinM2 = 6;
const int enPinM2 = 8;
const int s0Pin = 2;
const int s1Pin = 0;
const int s2aPin = 1;

const int s3aPin = ? ;
const int s4Pin =  ? ;
const int s5Pin =  ? ;

const int b1Pin = ?;
const int b2Pin = ?;
const int b3Pin = ?;

boolean s1 = false;
boolean s2 = false;
boolean atHome = false;
int val = 0;

int calculateSteps(int degrees, int driverPulsePerRev)
{
  int result = (degrees * (360 / driverPulsePerRev)); // main motor driverPulsePerRev should be se at 800
  return result;
}
void initializeM1ToHomePos()
{
  int stepsToHome = 0;
  int remainingStepsToHome = 0;
  digitalWrite(dirPinM1, LOW);
  while (atHome == false)
  {
    if (analogRead(s0Pin) == LOW)
    {
      atHome = true;
    }
    else
    {
      digitalWrite(stepPinM1, HIGH);
      delayMicroseconds(5000);
      digitalWrite(stepPinM1, LOW);
      delayMicroseconds(5000);
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
  if (analogRead(s2aPin) == HIGH)
  {
    digitalWrite(ledS2a, HIGH);
    s2aReady = true;
  }
  if (analogRead(s3aPin) == HIGH)
  {
    digitalWrite(ledS3a, HIGH);
    s3aReady = true;
  }
  if (analogRead(s4Pin) == HIGH)
  {
    s4Ready = true;
  }
  if (analogRead(s5Pin) == HIGH)
  {
    s5Ready = true;
  }
  if (s2aReady && s3aReady && s4Ready && s5Ready)
  {
    preCheckReady = true;
  }
  return preCheckReady;
}

void runMotorM2()
{
  digitalWrite(dirPinM2, HIGH);
  for (int x = 0; x < 1; x++)
  {

    digitalWrite(stepPinM2, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPinM2, LOW);
    delayMicroseconds(500);
  }
}
void runMotorM1()
{
  digitalWrite(dirPinM1, LOW);
  for (int x = 0; x < 1; x++)
  {
    turnDetected = true;
    previousPosition = rotaryPosition;
    rotaryPosition = rotaryPosition + 1;
    digitalWrite(stepPinM1, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPinM1, LOW);
    delayMicroseconds(500);
    if (rotaryPosition == 360)
    {
      rotaryPosition = 0; // made full circle reset position
    }
  }

  turnDetected = false;
}

void setup()
{
  //Decoder
  pinMode(decoderCLK, OUTPUT);
  pinMode(decoderDT, OUTPUT);

  //Motors
  pinMode(stepPinM1, OUTPUT);
  pinMode(dirPinM1, OUTPUT);
  pinMode(enPinM1, OUTPUT);
  pinMode(stepPinM2, OUTPUT);
  pinMode(dirPinM2, OUTPUT);
  pinMode(enPinM2, OUTPUT);
  digitalWrite(enPinM1, LOW);
  digitalWrite(enPinM2, LOW);

  //Sensors
  pinMode(s0Pin, INPUT);
  pinMode(s1Pin, INPUT);
  pinMode(s2aPin, INPUT);
  pinMode(s3aPin, INPUT);
  pinMode(s4Pin, INPUT);
  pinMode(s5Pin, INPUT);

  //LEDs
  pinMode(ledS2a, OUTPUT);
  pinMode(ledS3a, OUTPUT);
  
  //Buttons
  pinMode(b1Pin, OUTPUT);
  pinMode(b2Pin, OUTPUT);
  pinMode(b3Pin, OUTPUT);

}

void loop()
{
  if(b3State == HIGH){
    productionRun = false;
  }
  b1State = digitalRead(b1Pin);
  b2State = digitalRead(b2Pin);
  b3State = digitalRead(b3Pin);
  if (!productionRun)
  {

    if (b1State == HIGH)
    {
      readyToStart = preCheckCond();
      while (!readyToStart)
      {
        readyToStart = preCheckCond();
        if (readyToStart)
        {
          break;
        }
      }
      if (readyToStart)
      {
        initializeM1ToHomePos();
        // disable b1
        // enable b2
      }
      if(b2State == HIGH){
        //disable b2
        productionRun = true;
      }
    }
    initializeM1ToHomePos();
  }

  if (productionRun)
  {
    runMotorM1();
    if(rotaryPosition == 5){
      
    }
    if(rotaryPosition == 6){

    }
    if(rotaryPosition == 160){

    }
    if(rotaryPosition == 165){
      
    }
    if(rotaryPosition == 280){
      
    }
    if(rotaryPosition == 300){
      
    }
    if(rotaryPosition == 356){
      
    }

    // if (analogRead(s1Pin) == LOW)
    // {
    //   s1 = false;
    //   while (s1 == false)
    //   {
    //     runMotorM1();
    //     runMotorM2();
    //     if (analogRead(s2aPin) == LOW)
    //     {
    //       s1 = true;
    //     }
    //   }
    // }
  }
}