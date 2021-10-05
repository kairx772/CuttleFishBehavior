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
#define starting_point 200  // midpoint of a swings. Please check step_per_mm if the displacement is not the same as desired
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
int move_finished = 1; // Used to check if move is completed
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

  Serial.println("Stepper is Homing . . . . . . . . . . . ");

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
  Serial.println("Homing Completed");
  Serial.println("");
  stepperX.setMaxSpeed(MAX_SPEED);            // Set Max Speed of Stepper (Faster for regular movements)
  stepperX.setAcceleration(MAX_ACCEL);        // Set Acceleration of Stepper

}


void loop() {

  // Read input values from Serial
  // Command line input format: <amplitude> <max_speed> <frequency>

  while (!Serial.available() > 0)             // Holds the porgram here waiting for input

    digitalWrite(ENA, HIGH);
    // INPU FORMAT: <amplitude> <period> <initial_interval> <interval_increment>
    amplitude = Serial.parseFloat();
    Serial.print("> Amplitude (mm): ");
    while(amplitude < 0 || amplitude > MAX_POSITION){}  // Block the input if amplitude is not in proper value
    Serial.println(amplitude);                          // Print the rotation speed read from serial
    amplitude = amplitude * step_per_mm;                // Change into proper unit
    period = Serial.parseFloat();
    Serial.print("> Period (ms): ");
    Serial.println(period);                             //  Print the rotation period to serial
    interval = Serial.parseInt();
    Serial.print("> Initial Group interval(ms): ");
    Serial.println(interval);                           // Print the repeat times to serial
    interval_increment = Serial.parseInt();
    Serial.print("> Interval Increment(ms): ");
    Serial.println(interval_increment);                 // Print the interval time to serial (ms)

    // Check if input value fit in proper range

  //10 500 1 250

  move_finished = 0; // Set variable for checking move of the Stepper

  // Calculate max speed/accel in each run
  frequency = 1000 / period;
  Serial.print("> Frequency: ");
  Serial.println(frequency);                             
  max_speed_in_run = amplitude * frequency * 2 * PI;    
  Serial.print("> MAX_SPEED: ");
  Serial.println(max_speed_in_run);
  max_accel_in_run = amplitude * frequency * 2 * PI * frequency * 2 * PI;  
  Serial.print("> MAX_ACCEL: "); 
  Serial.println(max_accel_in_run);
  //Serial.println(step_per_mm);

  // Move the pole
  int swing_count = 5;
  int target_pos = 0;
  digitalWrite(ENA, HIGH);
  for (int j = 0; j < 3; j++) {         // interval incrementing between groups
    for (int i = 0; i < 2; i++) {       // run times in each group
      for (int k = 0; k < swing_count; k++) {               // One run back and forth
        stepperX.setMaxSpeed(max_speed_in_run);
        //Serial.println(max_speed_in_run);
        stepperX.setAcceleration(max_accel_in_run);
        //Serial.println(max_accel_in_run);

       //bort the run when the amplitude command is too large
       // if(starting_point + (amplitude / step_per_mm / 2) > MAX_POSITION){
       // Serial.println(starting_point + (amplitude / step_per_mm / 2));
        //Serial.println("Over Stroke Lenght! Please enter a smaller amplitude!");
        //break;
        //}

        
       stepperX.runToNewPosition(starting_point * step_per_mm + (amplitude / 2));
       delay(pause_time);
       stepperX.runToNewPosition(starting_point * step_per_mm - (amplitude / 2));
       delay(pause_time);
        
       // if(k < (swing_count/2)+1){
       //  target_pos = starting_point * step_per_mm + (amplitude/2 * (k+1) /(swing_count/2)) ;
       //   Serial.println(target_pos);
       //   stepperX.runToNewPosition(target_pos);
       //   target_pos = starting_point * step_per_mm - (amplitude/2 * (k+1) /(swing_count/2)) ;
       //   Serial.println(target_pos);
       //   stepperX.runToNewPosition(target_pos); 
        //}
     //else{
         // target_pos = starting_point * step_per_mm + (amplitude / 2) - (amplitude/2 * (k+1) /(swing_count/2)) ;
         // Serial.println(target_pos);
         // stepperX.runToNewPosition(target_pos);
         // target_pos = starting_point * step_per_mm - (amplitude / 2) + (amplitude/2 * (k+1) /(swing_count/2)) ;
         // Serial.println(target_pos);
         // stepperX.runToNewPosition(target_pos);
        //}

        
        Serial.print("swing! #");
        Serial.print(i);
        Serial.print("-");
        Serial.print(k);
        Serial.print(" interval: ");
        Serial.println(interval);
      }
      delay(interval);
    }
    interval += interval_increment;
  }
  stepperX.runToNewPosition(2);
  digitalWrite(ENA, LOW);

  

}
