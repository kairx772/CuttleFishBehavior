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
#define L_point 75
#define R_point 270
#define M_point 175
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
//int swing_count = 3;                            // swing count input default
int interval = 0;                               // swing interval input
int init_group_interval = 0;                    // interval between each group
int interval_increment = 0;                     // increment of interval between each groups
int repeat_times = 2;                           // how many times to repeat in each group
int group_count = 3;                            // how many groups do we want to do

float ampl_s = 5;
float ampl_m = 10;
float ampl_l = 20;
float ampl_xl = 25;

float max_speed_in_run = 0;       // max speed that would be calculated from A and w in each run
float max_accel_in_run = 0;       // max accel that would be calculated from A and w in each run

float moving_speed = 6200.0;
float swing_speed = 6200.0;
char cmd[1];

int swing_freq = 255;
int swing_period = 0;
int swing_count = 0;
int swap_delay = 500;

long L_rndpt = L_point;
long R_rndpt = R_point;



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
  amplitude = 100.0;
  stepperX.setMaxSpeed(6200.0);
  stepperX.setAcceleration(49000.0);
  // while (!Serial.available() > 0)             // Holds the porgram here waiting for input
  //unsigned int ser_avail = Serial.available();
  //char cmd = Serial.read();
  digitalWrite(ENA, HIGH);
  Serial.print("swing_period: ");

  //Serial.write("W");
  //while (Serial.available()>0) {}
  //swing_freq = Serial.parseInt();
  //swing_period = Serial.parseInt();
  

  while (1) {
    swing_count = 0;
    stepperX.setMaxSpeed(moving_speed);
    L_rndpt = random(L_point, (M_point-25));
    R_rndpt = random((M_point+25), R_point);

    delay(10);
    while (Serial.available()>0) {
      //cmd = Serial.read();
      Serial.readBytes(cmd, 1);
    }
    
    //cmd = Serial.read();
    //Serial.println("Wait...");
    if (cmd[0] == 'Z') {
      //Serial.println("waiting input");
      while (!Serial.available() > 0) {}
      swing_freq = Serial.parseInt();
      swing_period = Serial.parseInt();
      swap_delay = Serial.parseInt();
      //Serial.print("swing_freq: ");
      //Serial.println(swing_freq);
      //Serial.print("swing_period: ");
      //Serial.println(swing_period);
      cmd[0] = 'Y';
      
    } else if (cmd[0] == 'L') {
      Serial.println("Move to Left!!");
      stepperX.runToNewPosition(L_point * step_per_mm);
      
      Serial.flush();
      while(Serial.read()>= 0){} // clean the serial buffer
      //delay(100);
      while (!Serial.available() > 0) {} // wait till read from serial
      while (Serial.available()>0) {     // read from serial
        cmd[0] = Serial.read();
      }

    } else if (cmd[0] == 'R') {
      //Serial.println("Move to Right!!");
      stepperX.runToNewPosition((R_point) * step_per_mm);
      Serial.flush();
      while(Serial.read()>= 0){}
      //delay(100);
      while (!Serial.available() > 0) {}
      while (Serial.available()>0) {
        cmd[0] = Serial.read();
      }
      
    } else if (cmd[0] == 'O') {
      stepperX.runToNewPosition(2);
      Serial.flush();
      while(Serial.read()>= 0){}
      //delay(100);
      while (!Serial.available() > 0) {}
      while (Serial.available()>0) {
        cmd[0] = Serial.read();
      }
      
    } else if (cmd[0] == 'M') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      Serial.flush();
      while(Serial.read()>= 0){}
      //delay(100);
      while (!Serial.available() > 0) {}
      while (Serial.available()>0) {
        cmd[0] = Serial.read();
      }
    }
    else if (cmd[0] == 'A') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'A') {
        stepperX.runToNewPosition((L_point+ampl_s)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'A') {
          break;
        }
        stepperX.runToNewPosition((L_point-ampl_s)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    else if (cmd[0] == 'B') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'B') {
        stepperX.runToNewPosition((R_point+ampl_s)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'B') {
          break;
        }
        stepperX.runToNewPosition((R_point-ampl_s)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    else if (cmd[0] == 'C') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'C') {
        stepperX.runToNewPosition((L_point+ampl_m)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'C') {
          break;
        }
        stepperX.runToNewPosition((L_point-ampl_m)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    else if (cmd[0] == 'D') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'D') {
        stepperX.runToNewPosition((R_point+ampl_m)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'D') {
          break;
        }
        stepperX.runToNewPosition((R_point-ampl_m)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    else if (cmd[0] == 'E') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'E') {
        stepperX.runToNewPosition((L_point+ampl_l)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'E') {
          break;
        }
        stepperX.runToNewPosition((L_point-ampl_l)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    else if (cmd[0] == 'F') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'F') {
        stepperX.runToNewPosition((R_point+ampl_l)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'F') {
          break;
        }
        stepperX.runToNewPosition((R_point-ampl_l)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    else if (cmd[0] == 'G') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'G') {
        stepperX.runToNewPosition((L_point+ampl_xl)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'G') {
          break;
        }
        stepperX.runToNewPosition((L_point-ampl_xl)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    else if (cmd[0] == 'H') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'H') {
        stepperX.runToNewPosition((R_point+ampl_xl)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'H') {
          break;
        }
        stepperX.runToNewPosition((R_point-ampl_xl)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'a') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'a') {
        stepperX.runToNewPosition((L_rndpt+ampl_s)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'a') {
          break;
        }
        stepperX.runToNewPosition((L_rndpt-ampl_s)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'b') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'b') {
        stepperX.runToNewPosition((R_rndpt+ampl_s)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'b') {
          break;
        }
        stepperX.runToNewPosition((R_rndpt-ampl_s)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'c') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'c') {
        stepperX.runToNewPosition((L_rndpt+ampl_m)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'c') {
          break;
        }
        stepperX.runToNewPosition((L_rndpt-ampl_m)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'd') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'd') {
        stepperX.runToNewPosition((R_rndpt+ampl_m)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'd') {
          break;
        }
        stepperX.runToNewPosition((R_rndpt-ampl_m)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'e') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'e') {
        stepperX.runToNewPosition((L_rndpt+ampl_l)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'e') {
          break;
        }
        stepperX.runToNewPosition((L_rndpt-ampl_l)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'f') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'f') {
        stepperX.runToNewPosition((R_rndpt+ampl_l)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        if (cmd[0] != 'f') {
          break;
        }
        stepperX.runToNewPosition((R_rndpt-ampl_l)* step_per_mm);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'l') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'l') {
        L_rndpt = random(L_point-ampl_l, (M_point-25));
        stepperX.runToNewPosition((L_rndpt)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }

    else if (cmd[0] == 'r') {
      stepperX.runToNewPosition(M_point* step_per_mm);
      delay(swap_delay);
      while (cmd[0] == 'r') {
        R_rndpt = random(M_point+25, R_point-ampl_l);
        stepperX.runToNewPosition((R_rndpt)* step_per_mm);
        stepperX.setMaxSpeed(swing_speed);
        while (Serial.available()>0) {
          cmd[0] = Serial.read();
        }
        swing_count += 1;
        if (swing_count == swing_freq) {
          delay(swing_period);
          swing_count = 0;
        }
      }
    }
    
    else if (cmd[0] == 'X') {
      stepperX.runToNewPosition(2);
      delay(30000);
    }
    Serial.flush();
    while(Serial.read()>= 0){}
  }


  stepperX.runToNewPosition(2);
  digitalWrite(ENA, LOW);
  
}
