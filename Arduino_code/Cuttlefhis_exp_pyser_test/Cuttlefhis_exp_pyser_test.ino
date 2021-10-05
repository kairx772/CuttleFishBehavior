/*  Cuttelfish Experiment System v6 firmwate

  Created by Yen Chen Lin
  v6 is a system driven large stepper motor and ball screws with DM542 driver
  This program is for generating STEP/DIR control signal, 
  which would go through the amplifier to get high current and control the driver.

  unit: mm somteshit ms ms 
  Test: 20 1000      1 250
  
  二檔
  10 500 7000 3000
  20 800 7000 3000 
  30 1200 1 500
  30 1500 7000 3000
  50 2500 7000 3000

10 400 1 1

  150 6000 1 1

80 2000 2000
  一檔
  10 1000 1 10         
  10 500 1 250
  10 300 1 250


  
  try not to set the period less than 500
  
*/
#define MAX_POSITION 300
#define MAX_SPEED 100.0
#define MAX_ACCEL 100.0
#define mm_per_rev 10       // Depends on the screw you choose
#define pulse_per_rev 400   // Check with the DIP settings on motor driver
#define starting_point 150  // midpoint of a swings. Please check step_per_mm if the displacement is not the same as desired
#define L_point 95
#define R_point 245
#define M_point 170 
#define pause_time 0

#include "AccelStepper.h"
// Library created by Mike McCauley at http://www.airspayce.com/mikem/arduino/AccelStepper/

// AccelStepper Setup
AccelStepper stepperX(1, 7, 6);   // 1 = Easy Driver interface
// UNO Pin 2 connected to STEP amplifier then to the STEP pin of DM542 Driver
// UNO Pin 3 connected to DIR amplifier then to the DIR pin of DM542 Driver

// Define the Pins used
#define home_switch 4 // Pin 4 connected to Home Switch (MicroSwitch)
#define end_switch 5 // Pin 5 connected to Home Switch (MicroSwitch)
#define ENA 3 // Pin 3 connected to ENA on the driver

// unit calculation
int step_per_mm = pulse_per_rev/mm_per_rev;

// Stepper Travel Variables
long TravelX;  // Used to store the X value entered in the Serial Monitor
long initial_homing = 1; // Used to Home Stepper at startup

float amplitude = 100;                          // rotation amplitude input default
float period = 500;                             // swing period input (ms) default
double frequency = 1 / period;                  // swing frequency that would be calculated (1/ms)
int swing_count = 3;                            // swing count input default
int interval = 0;                               // swing interval input
int init_group_interval = 0;                    // interval between each group
int interval_increment = 0;                     // increment of interval between each groups
int repeat_times = 2;                           // how many times to repeat in each group
int group_count = 3;                            // how many groups do we want to do

float max_speed_in_run = 0;       // max speed that would be calculated from A and w in each run
float max_accel_in_run = 0;       // max accel that would be calculated from A and w in each run

void setup() {
  // Begin Serial communication
  Serial.begin(9600);  // Start the Serial monitor with speed of 9600 Bauds

  pinMode(home_switch, INPUT_PULLUP);
  pinMode(ENA, OUTPUT);
  digitalWrite(ENA, HIGH);
  
  delay(5);  // Wait for Driver wake up


  // Set Max Speed and Acceleration of each Steppers at startup for homing

  stepperX.setMaxSpeed(5000.0);               // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(500.0);            // Set Acceleration of Stepper


  // Start Homing procedure of Stepper Motor at startup

  //Serial.println("Stepper is Homing . . . . . . . . . . . ");

  while (digitalRead(home_switch)) {          // Make the Stepper move CCW until the switch is activated
    stepperX.moveTo(initial_homing);          // Set the position to move to
    initial_homing--;                         // Decrease by 1 for next move if needed
    stepperX.run();                           // Start moving the stepper
    delay(5);
  }

  stepperX.setCurrentPosition(0);             // Set the current position as zero for now
  stepperX.setMaxSpeed(5000.0);               // Set Max Speed of Stepper (Slower to get better accuracy)
  stepperX.setAcceleration(500.0);            // Set Acceleration of Stepper
  initial_homing = 1;

  while (!digitalRead(home_switch)) {         // Make the Stepper move CW until the switch is deactivated
    stepperX.moveTo(initial_homing);
    stepperX.run();
    initial_homing++;
    delay(5);
  }

  stepperX.setCurrentPosition(0);
  stepperX.setMaxSpeed(MAX_SPEED);            // Set Max Speed of Stepper (Faster for regular movements)
  stepperX.setAcceleration(MAX_ACCEL);        // Set Acceleration of Stepper

  //Serial.println("Ready...");

}


void loop() {

  // Read input values from Serial
  // Command line input format: <amplitude> <max_speed> <frequency>
  amplitude = 30.0;
  stepperX.setMaxSpeed(6200.0);
  stepperX.setAcceleration(49000.0);
  // while (!Serial.available() > 0)             // Holds the porgram here waiting for input
  unsigned int ser_avail = Serial.available();
  digitalWrite(ENA, HIGH);

  unsigned int cmd = 0;

  while (ser_avail > 0) {
    cmd = Serial.read();
    //Serial.println("Wait...");
    if (cmd == 'L') {
      //Serial.println("Move to Left!!");
      stepperX.runToNewPosition(L_point * step_per_mm);
      Serial.flush();
      while(Serial.read()>= 0){}
      //delay(200);
    } else if (cmd == 'R') {
      //Serial.println("Move to Right!!");
      stepperX.runToNewPosition(R_point * step_per_mm);
      Serial.flush();
      while(Serial.read()>= 0){}
      //delay(200);
    } else if (cmd == 'O') {
      stepperX.runToNewPosition(2);
      Serial.flush();
      while(Serial.read()>= 0){}
      //delay(200);
    } else if (cmd == 'M') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      Serial.flush();
      while(Serial.read()>= 0){}
      //delay(200);
    }
    Serial.flush();
    while(Serial.read()>= 0){}
    delay(10);
  }


  stepperX.runToNewPosition(2);
  digitalWrite(ENA, LOW);
  
}
