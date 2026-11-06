/*
 * Motor Control Test (prep.ino)
 *
 * Tests both left and right motors on the DRV8833 driver and provides
 * basic movement functions to control the robot (forward, reverse,
 * turn left, turn right, brake, and coast).
 */

#include "AlashMotorControlLite.h"

// DRV8833 Motor Driver Pins (PWM pins on Arduino Uno)
const uint8_t PIN_MOTOR_L_IN1 = 5;   // Left motor AIN1
const uint8_t PIN_MOTOR_L_IN2 = 6;   // Left motor AIN2
const uint8_t PIN_MOTOR_R_IN1 = 9;   // Right motor BIN1
const uint8_t PIN_MOTOR_R_IN2 = 10;  // Right motor BIN2
const uint8_t PIN_DRV_EEP     = 4;   // Driver enable pin (HIGH = enabled)

AlashMotorControlLite motorLeft(PWM_PWM, PIN_MOTOR_L_IN1, PIN_MOTOR_L_IN2);
AlashMotorControlLite motorRight(PWM_PWM, PIN_MOTOR_R_IN1, PIN_MOTOR_R_IN2);

// Sets left and right wheel speeds independently (-100 to 100)
void driveMotors(int16_t leftSpeed, int16_t rightSpeed) {
  leftSpeed  = constrain(leftSpeed, -100, 100);
  rightSpeed = constrain(rightSpeed, -100, 100);
  motorLeft.setSpeed(leftSpeed);
  motorRight.setSpeed(rightSpeed);
}

void moveForward(int16_t speed) {
  driveMotors(speed, speed);
}

void moveBackward(int16_t speed) {
  driveMotors(-speed, -speed);
}

void turnRight(int16_t speed) {
  driveMotors(speed, -speed);
}

void turnLeft(int16_t speed) {
  driveMotors(-speed, speed);
}

void brakeMotors() {
  motorLeft.brake();
  motorRight.brake();
}

void stopMotors() {
  motorLeft.stop();
  motorRight.stop();
}

void setup() {
  Serial.begin(115200);

  // Enable DRV8833 driver
  pinMode(PIN_DRV_EEP, OUTPUT);
  digitalWrite(PIN_DRV_EEP, HIGH);

  brakeMotors();
  delay(1000);
}

void loop() {
  // 1. Drive forward
  moveForward(80);
  delay(1500);

  // 2. Active brake
  brakeMotors();
  delay(500);

  // 3. Drive backward
  moveBackward(80);
  delay(1500);

  // 4. Spin left in place
  turnLeft(70);
  delay(800);

  // 5. Spin right in place
  turnRight(70);
  delay(800);

  // 6. Coast stop
  stopMotors();
  delay(2000);
}
