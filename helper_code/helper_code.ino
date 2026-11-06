/*
 * ============================================================================
 * SUMO ROBOT COMPETITION - GUIDED HELPER TEMPLATE (HALF-DONE CODE)
 * ============================================================================
 * Hardware Summary:
 *   - MCU: Arduino Uno R3
 *   - Motor Driver: DRV8833 (AIN1=5, AIN2=6, BIN1=9, BIN2=10, EEP=4)
 *   - Line Sensors: 2x Pololu 2-Ch QTR on A0, A1 (Front-Left) & A2, A3 (Front-Right)
 *   - Ultrasonic Sonar: HC-SR04 (Trig=7, Echo=8)
 *   - IMU: MPU-6050 on I2C (SDA=A4, SCL=A5)
 *
 * What is ALREADY DONE for you in this file:
 *   1. All pin definitions, sensor setup, and the 5-second startup delay.
 *   2. Motor helper functions (`driveMotors`, `stopMotors`, `turnByAngleIMU`).
 *   3. Automatic reading of all 4 QTR sensors (`qtr.leftMax`, `qtr.rightMax`),
 *      ultrasonic distance (`opponentDistCm`), and IMU gyroscope yaw (`imu.yawDeg`).
 *   4. The overall state-machine structure in `loop()`.
 *
 * What YOU need to complete (look for the "TODO" sections below):
 *   - TODO 1: Tune the sensor thresholds & motor speeds for your arena/chassis.
 *   - TODO 2: Complete `evadeArenaEdge()` to reverse and turn away from the line.
 *   - TODO 3: Complete `checkArenaBoundary()` to detect when a sensor hits the edge.
 *   - TODO 4: Complete `engageOrSearchOpponent()` to charge or scan for the opponent.
 *   - BONUS TODO: Use IMU data in `handleIMUTactics()` for tilt/collision recovery.
 * ============================================================================
 */

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "AlashMotorControlLite.h"

// ============================================================================
// 1. PIN DEFINITIONS
// ============================================================================

// DRV8833 Motor Driver Pins (PWM pins on Uno)
const uint8_t PIN_MOTOR_L_IN1 = 5;   // Left Motor AIN1
const uint8_t PIN_MOTOR_L_IN2 = 6;   // Left Motor AIN2
const uint8_t PIN_MOTOR_R_IN1 = 9;   // Right Motor BIN1
const uint8_t PIN_MOTOR_R_IN2 = 10;  // Right Motor BIN2
const uint8_t PIN_DRV_EEP     = 4;   // Driver enable pin (HIGH = enabled)

// Pololu 2-Channel QTR Line Sensors (Analog pins A0 - A3)
const uint8_t PIN_QTR_L1 = A0;       // Front-Left Outer
const uint8_t PIN_QTR_L2 = A1;       // Front-Left Inner
const uint8_t PIN_QTR_R1 = A2;       // Front-Right Inner
const uint8_t PIN_QTR_R2 = A3;       // Front-Right Outer

// HC-SR04 Ultrasonic Distance Sensor
const uint8_t PIN_SONAR_TRIG = 7;
const uint8_t PIN_SONAR_ECHO = 8;


// ============================================================================
// 2. TUNABLE CONSTANTS & THRESHOLDS (TODO 1: TUNE THESE VALUES!)
// ============================================================================

// --- QTR Reflectance Threshold (0 = pure white, 1023 = pure black) ---
// TODO 1A: Open Serial Monitor (115200 baud) to check your QTR readings on the
// white arena vs. the black boundary line, then adjust this threshold.
const int BLACK_LINE_THRESHOLD   = 650;

// --- Ultrasonic Sonar Distances (in cm) ---
const int OPPONENT_MAX_DIST_CM   = 60;   // Max distance to consider an opponent detected
const int OPPONENT_RAM_DIST_CM   = 15;   // Close-range distance to switch to full ramming speed
const unsigned long SONAR_PING_MS = 35;  // Time between sonar pings (ms)

// --- Motor Speeds (-100 to 100) ---
// TODO 1B: Adjust speeds to balance aggression vs. staying inside the ring.
const int16_t SPEED_RAM          = 100;  // Full power when pushing opponent close-up
const int16_t SPEED_ATTACK       = 80;   // Closing speed when opponent is spotted
const int16_t SPEED_SEARCH_TURN  = 55;   // Spinning speed while searching for opponent
const int16_t SPEED_RETREAT      = -80;  // Reverse speed when backing away from the edge

// --- Timing Constants ---
const unsigned long RETREAT_REVERSE_MS = 300;  // How long (ms) to reverse from the edge
const unsigned long RETREAT_TURN_MS    = 350;  // Fallback turn time (ms) if IMU is not used
const unsigned long STARTUP_DELAY_MS   = 5000; // 5-second competition start delay


// ============================================================================
// 3. OBJECTS & SENSOR STATE (Pre-configured for you)
// ============================================================================

AlashMotorControlLite motorLeft(PWM_PWM, PIN_MOTOR_L_IN1, PIN_MOTOR_L_IN2);
AlashMotorControlLite motorRight(PWM_PWM, PIN_MOTOR_R_IN1, PIN_MOTOR_R_IN2);
Adafruit_MPU6050 mpu;

// Holds the latest reflectance readings (0 to 1023)
struct QTRReadings {
  int l1, l2, r1, r2;
  int leftMax;    // Highest reading on the Front-Left sensor (max of L1, L2)
  int rightMax;   // Highest reading on the Front-Right sensor (max of R1, R2)
  int overallMax; // Highest reading across all 4 channels
};

// Holds the latest MPU-6050 IMU readings
struct IMUState {
  bool available;
  float yawDeg;        // Integrated Z-axis heading angle in degrees
  float gyroZBias;     // Calibrated Z-axis gyro offset
  float accelX;        // Forward/backward acceleration (m/s^2)
  float accelY;        // Left/right acceleration (m/s^2)
  float accelZ;        // Vertical acceleration (m/s^2)
  unsigned long lastUpdateUs;
};

QTRReadings qtr;
IMUState imu = {false, 0.0f, 0.0f, 0.0f, 0.0f, 9.8f, 0};

int opponentDistCm = 999; // 999 = no opponent in range
unsigned long lastSonarTime = 0;


// ============================================================================
// 4. PROVIDED HELPER FUNCTIONS (Ready to use!)
// ============================================================================

/**
 * Sets left and right wheel speeds from -100 (full reverse) to +100 (full forward).
 */
void driveMotors(int16_t leftSpeed, int16_t rightSpeed) {
  leftSpeed  = constrain(leftSpeed, -100, 100);
  rightSpeed = constrain(rightSpeed, -100, 100);
  motorLeft.setSpeed(leftSpeed);
  motorRight.setSpeed(rightSpeed);
}

/**
 * Actively brakes both motors immediately.
 */
void stopMotors() {
  motorLeft.brake();
  motorRight.brake();
}

/**
 * Reads all 4 QTR channels and updates `qtr.leftMax`, `qtr.rightMax`, and `qtr.overallMax`.
 */
void updateQTRSensors() {
  qtr.l1 = analogRead(PIN_QTR_L1);
  qtr.l2 = analogRead(PIN_QTR_L2);
  qtr.r1 = analogRead(PIN_QTR_R1);
  qtr.r2 = analogRead(PIN_QTR_R2);

  qtr.leftMax    = max(qtr.l1, qtr.l2);
  qtr.rightMax   = max(qtr.r1, qtr.r2);
  qtr.overallMax = max(qtr.leftMax, qtr.rightMax);
}

/**
 * Pings the HC-SR04 ultrasonic sensor every SONAR_PING_MS and updates `opponentDistCm`.
 * (Returns 999 if no object is detected within ~85 cm).
 */
void updateSonar() {
  unsigned long now = millis();
  if (now - lastSonarTime < SONAR_PING_MS) return;
  lastSonarTime = now;

  digitalWrite(PIN_SONAR_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SONAR_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_SONAR_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_SONAR_ECHO, HIGH, 5000);
  if (duration == 0) {
    opponentDistCm = 999;
  } else {
    opponentDistCm = (int)(duration * 0.0343 / 2.0);
  }
}

/**
 * Initializes the MPU-6050 and calibrates gyro drift while stationary.
 */
void setupIMU() {
  if (!mpu.begin()) {
    Serial.println(F("[IMU] MPU6050 not found - using timed turns."));
    imu.available = false;
    return;
  }
  imu.available = true;
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  float sumZ = 0.0f;
  for (int i = 0; i < 50; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumZ += g.gyro.z;
    delay(5);
  }
  imu.gyroZBias = sumZ / 50.0f;
  imu.lastUpdateUs = micros();
  Serial.println(F("[IMU] Calibrated and ready."));
}

/**
 * Updates `imu.yawDeg` (heading angle) and `imu.accelX/Y/Z` every loop.
 */
void updateIMU() {
  if (!imu.available) return;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long nowUs = micros();
  float dt = (nowUs - imu.lastUpdateUs) / 1000000.0f;
  imu.lastUpdateUs = nowUs;

  float rateDegPerSec = (g.gyro.z - imu.gyroZBias) * 57.2957795f;
  if (abs(rateDegPerSec) > 0.5f) {
    imu.yawDeg += rateDegPerSec * dt;
  }

  imu.accelX = a.acceleration.x;
  imu.accelY = a.acceleration.y;
  imu.accelZ = a.acceleration.z;
}

/**
 * Spins the robot in place by `targetDeltaDeg` degrees using the IMU gyroscope
 * (or falls back to `RETREAT_TURN_MS` if the IMU isn't connected).
 *   - Positive angle (+deg) = Spin Left (CCW)
 *   - Negative angle (-deg) = Spin Right (CW)
 */
void turnByAngleIMU(float targetDeltaDeg, int16_t turnSpeed) {
  if (!imu.available) {
    if (targetDeltaDeg > 0) driveMotors(-turnSpeed, turnSpeed);
    else                    driveMotors(turnSpeed, -turnSpeed);
    delay(RETREAT_TURN_MS);
    stopMotors();
    return;
  }

  updateIMU();
  float startYaw = imu.yawDeg;
  unsigned long startTime = millis();

  while (millis() - startTime < 800) { // 800ms safety timeout
    updateIMU();
    if (abs(imu.yawDeg - startYaw) >= abs(targetDeltaDeg)) {
      break;
    }
    if (targetDeltaDeg > 0) {
      driveMotors(-turnSpeed, turnSpeed); // Spin Left
    } else {
      driveMotors(turnSpeed, -turnSpeed); // Spin Right
    }
  }
  stopMotors();
}


// ============================================================================
// 5. STUDENT MISSION LOGIC (COMPLETE THE TODOs BELOW!)
// ============================================================================

/**
 * TODO 2: Evade the arena boundary line!
 * Called when `checkArenaBoundary()` detects the black line on the left or right.
 *
 * @param hitLeft  true if the front-left QTR sensor triggered
 * @param hitRight true if the front-right QTR sensor triggered
 */
void evadeArenaEdge(bool hitLeft, bool hitRight) {
  // Step 1: Brake immediately to stop forward momentum
  stopMotors();
  delay(40);

  // Step 2: Drive backward at SPEED_RETREAT for RETREAT_REVERSE_MS milliseconds
  // >>> WRITE YOUR CODE HERE <<<


  // Step 3: Turn away from the edge toward the center of the ring!
  // Hint: You can use `turnByAngleIMU(angle, SPEED_SEARCH_TURN)`:
  //   - If ONLY `hitLeft` is true, turn right (e.g., -135 degrees).
  //   - If ONLY `hitRight` is true, turn left (e.g., +135 degrees).
  //   - If BOTH hit head-on, turn 180 degrees.
  // >>> WRITE YOUR CODE HERE <<<

}

/**
 * TODO 3: Check if the robot has reached the black boundary line.
 * Returns `true` if an edge was detected and `evadeArenaEdge()` was called,
 * or `false` if the robot is still safely inside the ring.
 */
bool checkArenaBoundary() {
  // Step 1: Compare `qtr.leftMax` and `qtr.rightMax` against `BLACK_LINE_THRESHOLD`
  bool hitLeft  = false; // <-- TODO: Replace `false` with your comparison!
  bool hitRight = false; // <-- TODO: Replace `false` with your comparison!

  // Step 2: If either side hit the black line, call `evadeArenaEdge(hitLeft, hitRight)`
  // and return `true`.
  if (hitLeft || hitRight) {
    // >>> WRITE YOUR CODE HERE <<<

    return true;
  }

  return false;
}

/**
 * TODO 4: Search for the opponent and attack!
 * Uses `opponentDistCm` (updated automatically from the ultrasonic sensor).
 */
void engageOrSearchOpponent() {
  // Check if an opponent is within range (between 1 cm and OPPONENT_MAX_DIST_CM)
  bool opponentDetected = (opponentDistCm > 0 && opponentDistCm <= OPPONENT_MAX_DIST_CM);

  if (opponentDetected) {
    // --- ATTACK MODE ---
    // TODO 4A: Drive straight toward the opponent!
    //   - If `opponentDistCm <= OPPONENT_RAM_DIST_CM`, charge at full `SPEED_RAM`.
    //   - Otherwise, close in at `SPEED_ATTACK`.
    //   - Optional Bonus: Use `imu.yawDeg` to apply a small steering correction so
    //     both wheels track dead-straight while ramming.
    // >>> WRITE YOUR CODE HERE <<<


  } else {
    // --- SEARCH MODE ---
    // TODO 4B: No opponent in front of us! Spin in place (or curve around the ring)
    // using `driveMotors(...)` and `SPEED_SEARCH_TURN` until the sonar spots them.
    // >>> WRITE YOUR CODE HERE <<<


  }
}

/**
 * BONUS TODO (Optional): Use the MPU-6050 Accelerometer for tactical maneuvers!
 *   - Anti-Wedge: If `imu.accelZ < 7.5` and `abs(imu.accelX) > 4.5`, our front is lifted!
 *   - Side Impact: If no opponent is in front (`opponentDistCm > OPPONENT_MAX_DIST_CM`)
 *     and `abs(imu.accelY) > 13.0`, we were rammed from the side—spin 90 deg toward them!
 * Returns `true` if a maneuver was triggered.
 */
bool handleIMUTactics() {
  if (!imu.available) return false;

  // >>> OPTIONAL: WRITE YOUR BONUS IMU LOGIC HERE <<<

  return false;
}


// ============================================================================
// 6. SETUP & MAIN CONTROL LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);

  // Enable the DRV8833 motor driver
  pinMode(PIN_DRV_EEP, OUTPUT);
  digitalWrite(PIN_DRV_EEP, HIGH);

  // Configure Ultrasonic Sonar pins
  pinMode(PIN_SONAR_TRIG, OUTPUT);
  pinMode(PIN_SONAR_ECHO, INPUT);

  // Configure QTR Line Sensor pins
  pinMode(PIN_QTR_L1, INPUT);
  pinMode(PIN_QTR_L2, INPUT);
  pinMode(PIN_QTR_R1, INPUT);
  pinMode(PIN_QTR_R2, INPUT);

  stopMotors();
  setupIMU();

  Serial.println(F("Robot initialized. Waiting 5 seconds before match start..."));
  delay(STARTUP_DELAY_MS);
  imu.lastUpdateUs = micros();
  Serial.println(F("GO!"));
}

void loop() {
  // 1. Update all sensors first
  updateQTRSensors();
  updateIMU();

  // 2. HIGHEST PRIORITY: Stay inside the arena!
  //    If a black boundary line is detected, evade immediately and restart the loop.
  if (checkArenaBoundary()) {
    return;
  }

  // 3. Ping the ultrasonic sensor to measure distance to the opponent
  updateSonar();

  // 4. OPTIONAL BONUS: Check IMU for tilt/wedge recovery or side impacts
  if (handleIMUTactics()) {
    return;
  }

  // 5. Search for the opponent or charge at them!
  engageOrSearchOpponent();
}
