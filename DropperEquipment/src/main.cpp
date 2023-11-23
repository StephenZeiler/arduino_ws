#include <Arduino.h>
#include <Stepper.h>

// put function declarations here:
int myFunction(int, int);
long motorSpeed = 10;
Stepper myStepper(2048, 9, 11, 10, 12); //RPM,IN1,IN3,IN2,IN4
const int buttonPin = 2;  // the number of the pushbutton pin

int buttonState = 0;  // variable for reading the pushbutton status

void setup() {
  pinMode(buttonPin, INPUT);
myStepper.setSpeed(motorSpeed);
//myStepper.step(2048); 1024 rotates the stepper to 180 degrees, 2048 is a full circle. A neg number will rotate opposite.

  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  buttonState = digitalRead(buttonPin); //button pin
  if (buttonState == HIGH) {
  myStepper.step(1024);
  delay(500);
  myStepper.step(-1024);
  delay(500);
    // turn motor on:
    
  } 
  else{
    
  }

}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}