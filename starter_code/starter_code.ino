/*
 * UCL Sumo Robot Competition - Starter Code (Arduino Uno R3)
 *
 * This file contains the pin mappings, sensor setup, and basic motor functions
 * for your sumo robot. We have included a simple baseline loop at the bottom
 * to get your robot moving. Your goal is to modify loop() to improve how your
 * robot stays inside the ring and tracks down the opponent.
 *
 * Required Libraries (install via Tools -> Manage Libraries in Arduino IDE):
 *   - AlashMotorControlLite
 *   - Adafruit MPU6050 (click "Install All" when asked about dependencies)
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "AlashMotorControlLite.h"

// ----------------------------------------------------------------------------
// 1. Pin Definitions
// ----------------------------------------------------------------------------

// DRV8833 Motor Driver (Needs PWM pins on the Uno: 3, 5, 6, 9, 10, or 11)
const uint8_t PIN_MOTOR_L_IN1 = 5;   // Left motor AIN1
const uint8_t PIN_MOTOR_L_IN2 = 6;   // Left motor AIN2
const uint8_t PIN_MOTOR_R_IN1 = 9;   // Right motor BIN1
const uint8_t PIN_MOTOR_R_IN2 = 10;  // Right motor BIN2
const uint8_t PIN_DRV_EEP     = 4;   // Driver sleep/enable pin (HIGH = enabled)

// Pololu 2-Channel QTR Line Sensors (Analog pins A0 - A3)
const uint8_t PIN_QTR_L1 = A0;       // Front-left outer sensor
const uint8_t PIN_QTR_L2 = A1;       // Front-left inner sensor
const uint8_t PIN_QTR_R1 = A2;       // Front-right inner sensor
const uint8_t PIN_QTR_R2 = A3;       // Front-right outer sensor

// HC-SR04 Ultrasonic Distance Sensor
const uint8_t PIN_SONAR_TRIG = 7;
const uint8_t PIN_SONAR_ECHO = 8;

// MPU-6050 IMU uses the hardware I2C pins on the Uno:
// SDA -> A4, SCL -> A5


// ----------------------------------------------------------------------------
// 2. Settings & Hardware Objects
// ----------------------------------------------------------------------------

// Tune this threshold after checking your sensor readings in the Serial Monitor.
// Analog readings go from 0 (bright white) to 1023 (dark black).
const int BLACK_THRESHOLD = 650;

// Maximum distance (in cm) to treat a sonar reading as an opponent
const int MAX_OPPONENT_DIST_CM = 60;

// Standard 5-second delay before the match starts
const unsigned long STARTUP_DELAY = 5000;

AlashMotorControlLite motorLeft(PWM_PWM, PIN_MOTOR_L_IN1, PIN_MOTOR_L_IN2);
AlashMotorControlLite motorRight(PWM_PWM, PIN_MOTOR_R_IN1, PIN_MOTOR_R_IN2);

Adafruit_MPU6050 mpu;
bool imuAvailable = false;


// ----------------------------------------------------------------------------
// 3. Helper Functions
// ----------------------------------------------------------------------------

// Sets the speed of the left and right wheels (-100 to 100).
// Negative values spin the wheel in reverse; positive values spin it forward.
void driveMotors(int16_t leftSpeed, int16_t rightSpeed) {
  leftSpeed  = constrain(leftSpeed, -100, 100);
  rightSpeed = constrain(rightSpeed, -100, 100);
  motorLeft.setSpeed(leftSpeed);
  motorRight.setSpeed(rightSpeed);
}

// Brakes both motors immediately.
void stopMotors() {
  motorLeft.brake();
  motorRight.brake();
}

// Reads a single QTR sensor channel (0 = white, 1023 = black).
int readLineSensor(uint8_t pin) {
  return analogRead(pin);
}

// Triggers the ultrasonic sensor and returns the distance in cm.
// Returns 999 if nothing is detected within ~85 cm.
int readDistanceCm() {
  digitalWrite(PIN_SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_SONAR_TRIG, LOW);

  // 5000 us timeout (~85 cm) keeps the code from hanging if no echo comes back
  unsigned long duration = pulseIn(PIN_SONAR_ECHO, HIGH, 5000);
  if (duration == 0) {
    return 999;
  }
  return (int)(duration * 0.0343 / 2.0);
}


// ----------------------------------------------------------------------------
// 4. Setup (Runs once on power-up / reset)
// ----------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  // Wake up the DRV8833 motor driver
  pinMode(PIN_DRV_EEP, OUTPUT);
  digitalWrite(PIN_DRV_EEP, HIGH);

  // Ultrasonic sensor pins
  pinMode(PIN_SONAR_TRIG, OUTPUT);
  pinMode(PIN_SONAR_ECHO, INPUT);

  // Line sensor pins
  pinMode(PIN_QTR_L1, INPUT);
  pinMode(PIN_QTR_L2, INPUT);
  pinMode(PIN_QTR_R1, INPUT);
  pinMode(PIN_QTR_R2, INPUT);

  stopMotors();

  // Check if the MPU-6050 IMU is wired up
  if (mpu.begin()) {
    imuAvailable = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 connected.");
  } else {
    Serial.println("MPU6050 not found - running without IMU.");
  }

  Serial.println("Waiting 5 seconds before match start...");
  delay(STARTUP_DELAY);
  Serial.println("Starting!");
}


// ----------------------------------------------------------------------------
// 5. Main Loop (Write your control logic here)
// ----------------------------------------------------------------------------

void loop() {
  // 1. Read the line sensors and ultrasonic sensor
  int leftLine   = max(readLineSensor(PIN_QTR_L1), readLineSensor(PIN_QTR_L2));
  int rightLine  = max(readLineSensor(PIN_QTR_R1), readLineSensor(PIN_QTR_R2));
  int distanceCm = readDistanceCm();

  // Optional: read accelerometer and gyroscope values if you are using the IMU
  sensors_event_t accel, gyro, temp;
  if (imuAvailable) {
    mpu.getEvent(&accel, &gyro, &temp);
    // accel.acceleration.x, .y, .z (in m/s^2)
    // gyro.gyro.x, .y, .z          (in rad/s)
  }

  // 2. Boundary check: if either front sensor sees the black line, back up and turn
  if (leftLine > BLACK_THRESHOLD || rightLine > BLACK_THRESHOLD) {
    stopMotors();
    delay(50);

    driveMotors(-70, -70); // Back up
    delay(300);

    driveMotors(60, -60);  // Turn right
    delay(350);
    return;
  }

  // 3. Search and attack: drive forward if an opponent is in front, otherwise spin to search
  if (distanceCm <= MAX_OPPONENT_DIST_CM) {
    driveMotors(80, 80);
  } else {
    driveMotors(50, -50);
  }
}
