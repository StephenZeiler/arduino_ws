#include <Arduino.h>
#include <Stepper.h>

//Ram
//const int ramPin = ?;

//Air Release
//const int airReleasePin = ?;

//Encoder
const int s0CLKPin = 12;
const int s0DTPin = 13;

//Motors
const int stepPinM1 = 11; // 11 is pin location
const int dirPinM1 = 10;
const int enPinM1 = 9;
const int stepPinM2 = 6; // 5 is pin location
const int dirPinM2 = 5;
const int enPinM2 = 8;
const int stepPinM3 = 3;//was 4
const int dirPinM3 = 7; //was 3
const int enPinM3 = 2;

//Sensors

const int s1Pin = 0;
const int s2aPin = 8;
const int s2bPin = 1;
const int s3aPin = 12;
const int s3bPin = 5;
const int s4Pin =  9;
const int s5Pin =  13;
const int s6Pin =  2;
const int s7Pin =  6;

//Buttons
const int homeButtonPin = A14;
const int startButtonPin = A11;
const int stopButtonPin = A10;

bool readyToStart = false;
bool productionRun = false;
volatile boolean turnDetected;
int rotaryPosition;
int previousPosition;
int stepsToTake;
bool s1 = false;
bool s2 = false;
bool preCheckPass = false;
int val = 0;
 int m1Step = 1;
 int m2Step = 1;
 int m3Step = 1;
 long previousM1Micros = 0;  
 long previousM2Micros = 0;  
long previousM3Micros = 0;  
long m1Speed = 75; 
long m2Speed = 100; 
long m3Speed = 150; 

int encoderCurrentState;
int encoderPreviousState;
int encoderCount = 0;

void isr(){
  delay(4);
  if(digitalRead(s0CLKPin)){
    
  }
}
int calculateSteps(int degrees, int driverPulsePerRev)
{
  int result = (degrees * (360 / driverPulsePerRev)); // main motor driverPulsePerRev should be se at 6400
  return result;
}
void initializeM1ToHomePos()
{
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
      delayMicroseconds(1000);
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
 // while(!preCheckReady){ // remove loop and just return whether it is ready or not....
  if (analogRead(s2aPin) == HIGH)
  {
    s2aReady = true;
  }
    if (analogRead(s3aPin) == HIGH)
    {
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
  //}
  return preCheckReady;
}
void actuateAirRelease()
{
  for (int x = 0; x < 1; x++)
  {
    //digitalWrite(airReleasePin, HIGH);
    delayMicroseconds(500);
    //digitalWrite(airReleasePin, LOW);
    delayMicroseconds(500);
  }
}
void actuateAirRam()
{
  for (int x = 0; x < 1; x++)
  {
   // digitalWrite(ramPin, HIGH);
    delayMicroseconds(500);
   // digitalWrite(ramPin, LOW);
    delayMicroseconds(500);
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
  unsigned long currentMicros = micros();
  //digitalWrite(dirPinM1, LOW);
  for (int x = 0; x < 1; x++)
  {
    if((currentMicros - previousM1Micros)> m1Speed)
     { // Moved down here where it belongs: Got ya.
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
    if (rotaryPosition == 360)
    {
      rotaryPosition = 0; // made full circle reset position
    }
  }
}
void encoderCheck(){
  encoderCurrentState = digitalRead(s0CLKPin);
  if(encoderCurrentState != encoderPreviousState){
    encoderCount++;
  }
  encoderPreviousState = encoderCurrentState;
}
void setup()
{
  //Decoder
  pinMode(s0CLKPin, INPUT);
  pinMode(s0DTPin, INPUT);
  encoderPreviousState = digitalRead(s0CLKPin);

  //Air release
  //pinMode(airReleasePin, OUTPUT);

  //Ram
  //pinMode(ramPin, OUTPUT);

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

  //Sensors
  pinMode(s0DTPin, INPUT);
  pinMode(s1Pin, INPUT);
  pinMode(s2aPin, INPUT);
  pinMode(s3aPin, INPUT);
  pinMode(s4Pin, INPUT);
  pinMode(s5Pin, INPUT);

  //LEDs
  //pinMode(ledS2a, OUTPUT);
  //pinMode(ledS3a, OUTPUT);
  
  //Buttons
  pinMode(homeButtonPin, OUTPUT);
  pinMode(startButtonPin, OUTPUT);
  pinMode(stopButtonPin, OUTPUT);
  // pinMode(b2Pin, OUTPUT);
  // pinMode(b3Pin, OUTPUT);
}

void loop()
{
  encoderCheck();
  int homeButtonState = digitalRead(homeButtonPin);
  int startButtonState = digitalRead(startButtonPin);
  int stopButtonState = digitalRead(stopButtonPin);
 // bool preStartReady = ;
  unsigned long currentMicros = micros();
  if(homeButtonState==HIGH && !readyToStart){
   // if(preCheckCond()){

      readyToStart = true;
      initializeM1ToHomePos();
    //}
  }
  if(startButtonState == HIGH && readyToStart){
    productionRun = true;
  }
  if(startButtonState == HIGH){
    productionRun = true;
    
  }
  if(stopButtonState==HIGH){
    productionRun = false;
    readyToStart = false;
  }
  if(productionRun && encoderCount <1){
    runMotorM1();

  }

  // if((currentMicros - previousM1Micros)> m1Speed)
  // { // Moved down here where it belongs: Got ya.
  // runMotorM1();
  // previousM1Micros = currentMicros; 
  // }
   
  //  if((currentMicros - previousM3Micros)> m3Speed)
  // { // Moved down here where it belongs: Got ya.
  // previousM3Micros = currentMicros; 
  // runMotorM3();
  
    
  //}
  
  //runMotorM2();
  //runMotorM3();
  // if(b3State == HIGH){
  //   productionRun = false;
  // }
  // b1State = digitalRead(b1Pin);
  // b2State = digitalRead(b2Pin);
  // b3State = digitalRead(b3Pin);
  // if (!productionRun)
  // {

  //   if (b1State == HIGH)
  //   {
  //     readyToStart = preCheckCond();
  //     while (!readyToStart)
  //     {
  //       readyToStart = preCheckCond();
  //       if (readyToStart)
  //       {
  //         break;
  //       }
  //     }
  //     if (readyToStart)
  //     {
  //       initializeM1ToHomePos();
  //       // disable b1
  //       // enable b2
  //     }
  //     if(b2State == HIGH && readyToStart){
  //       //disable b2
  //       productionRun = true;
  //     }
  //   }
  //   initializeM1ToHomePos();
  // }

  // if (productionRun)
  // {
  //   runMotorM1();
  //   if(rotaryPosition == 5){
  //      runMotorM2();
  //   }
  //   if(rotaryPosition == 6){
  //     runMotorM3();
  //   }
  //   if(rotaryPosition == 160){

  //   }
  //   if(rotaryPosition == 165){
  //     actuateAirRam();
  //   }
  //   if(rotaryPosition == 280){
      
  //   }
  //   if(rotaryPosition == 300){
      
  //   }
  //   if(rotaryPosition == 356){
      
  //   }




///////////////////////////
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
 // }
}