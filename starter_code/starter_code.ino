/*
 * Sumo Robot Competition - Starter Code (Arduino Uno R3)
 *
 * This file contains the pin mappings, sensor setup, and basic motor functions
 * for your sumo robot. Modify loop() to implement your robot's behaviour.
 *
 * Required Libraries (install via Tools -> Manage Libraries in Arduino IDE):
 *   - AlashMotorControlLite
 *   - Adafruit MPU6050
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "AlashMotorControlLite.h"

// ----------------------------------------------------------------------------
// 1. Pin Definitions
// ----------------------------------------------------------------------------

// DRV8833 Motor Driver (PWM pins on Uno: 3, 5, 6, 9, 10, 11)
const uint8_t PIN_MOTOR_L_IN1 = 5;
const uint8_t PIN_MOTOR_L_IN2 = 6;
const uint8_t PIN_MOTOR_R_IN1 = 9;
const uint8_t PIN_MOTOR_R_IN2 = 10;
const uint8_t PIN_DRV_EEP     = 4;

// Pololu 2-Channel QTR Line Sensors (Analog pins A0 - A3)
const uint8_t PIN_QTR_L1 = A0;
const uint8_t PIN_QTR_L2 = A1;
const uint8_t PIN_QTR_R1 = A2;
const uint8_t PIN_QTR_R2 = A3;

// HC-SR04 Ultrasonic Distance Sensor
const uint8_t PIN_SONAR_TRIG = 7;
const uint8_t PIN_SONAR_ECHO = 8;

// MPU-6050 IMU uses hardware I2C: SDA -> A4, SCL -> A5


// ----------------------------------------------------------------------------
// 2. Objects & Constants
// ----------------------------------------------------------------------------

const unsigned long STARTUP_DELAY = 5000;

AlashMotorControlLite motorLeft(PWM_PWM, PIN_MOTOR_L_IN1, PIN_MOTOR_L_IN2);
AlashMotorControlLite motorRight(PWM_PWM, PIN_MOTOR_R_IN1, PIN_MOTOR_R_IN2);

Adafruit_MPU6050 mpu;
bool imuAvailable = false;


// ----------------------------------------------------------------------------
// 3. Helper Functions
// ----------------------------------------------------------------------------

// Sets left and right motor speeds (-100 to 100).
void driveMotors(int16_t leftSpeed, int16_t rightSpeed) {
  leftSpeed  = constrain(leftSpeed, -100, 100);
  rightSpeed = constrain(rightSpeed, -100, 100);
  motorLeft.setSpeed(leftSpeed);
  motorRight.setSpeed(rightSpeed);
}

// Brakes both motors.
void stopMotors() {
  motorLeft.brake();
  motorRight.brake();
}

// Reads a single QTR sensor channel (0 to 1023).
int readLineSensor(uint8_t pin) {
  return analogRead(pin);
}

// Reads distance from the ultrasonic sensor in cm (returns 999 if out of range).
int readDistanceCm() {
  digitalWrite(PIN_SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_SONAR_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_SONAR_ECHO, HIGH, 5000);
  if (duration == 0) {
    return 999;
  }
  return (int)(duration * 0.0343 / 2.0);
}


// ----------------------------------------------------------------------------
// 4. Setup
// ----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  pinMode(PIN_DRV_EEP, OUTPUT);
  digitalWrite(PIN_DRV_EEP, HIGH);

  pinMode(PIN_SONAR_TRIG, OUTPUT);
  pinMode(PIN_SONAR_ECHO, INPUT);

  pinMode(PIN_QTR_L1, INPUT);
  pinMode(PIN_QTR_L2, INPUT);
  pinMode(PIN_QTR_R1, INPUT);
  pinMode(PIN_QTR_R2, INPUT);

  stopMotors();

  if (mpu.begin()) {
    imuAvailable = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  delay(STARTUP_DELAY);
}


// ----------------------------------------------------------------------------
// 5. Main Loop
// ----------------------------------------------------------------------------

void loop() {
  // Read sensors and write your robot control logic here
}
