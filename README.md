# Sumo Robot Competition — Participant Manual

This repository contains the wiring guide, component datasheets, and starter code for the UCL Sumo Robot Workshop.

## How the Workshop Works

1. **Select a Chassis:** Choose one of the pre-made CAD chassis designs.
2. **Mechanical Assembly:** Mount the two N20 gearmotors, wheels, front ultrasonic sensor, bottom line sensors, and the Arduino Uno + breadboard inside the chassis.
3. **Electronics & Wiring:** Connect the motors, driver, and sensors to the Arduino Uno following the pinout tables below.
4. **Testing & Programming:** Install the required libraries, upload [`starter_code/starter_code.ino`](starter_code/starter_code.ino) to check that your hardware works, and then write your own strategy in `loop()`.

---

## Repository Contents

* [`starter_code/starter_code.ino`](starter_code/starter_code.ino) — The main starter template for the competition. It sets up all the pins and gives you basic functions to read sensors and drive the motors.
* [`prep/prep.ino`](prep/prep.ino) — A short test script for checking a single motor channel on the DRV8833 driver.

---

## Parts List & Datasheets

| Component | Qty | Description | Supply Voltage | Datasheet / Documentation |
| :--- | :---: | :--- | :---: | :--- |
| **Arduino Uno R3** | 1 | Microcontroller board | 5V logic (7–12V VIN) | [Arduino Uno R3 Datasheet (PDF)](https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf) |
| **DRV8833 Motor Driver** | 1 | Dual H-bridge driver for controlling motor speed and direction | 2.7V – 10.8V (`VM`) | [TI DRV8833 Datasheet (PDF)](https://www.ti.com/lit/ds/symlink/drv8833.pdf) |
| **N20 DC Gearmotors** | 2 | Left and right drive motors | 6V | [Pololu Micro Metal Gearmotor Specs](https://www.pololu.com/category/60/micro-metal-gearmotors) |
| **Pololu 2-Ch QTR Sensor** | 2 | Infrared reflectance sensors mounted under the front edge (4 channels total) | 5V | [Pololu QTR User Guide](https://www.pololu.com/docs/0J19) |
| **HC-SR04 Ultrasonic Sensor** | 1 | Front-mounted distance sensor (2 cm – 400 cm range) | 5V | [HC-SR04 Datasheet (PDF)](https://cdn.sparkfun.com/datasheets/Sensors/Proximity/HCSR04.pdf) |
| **MPU-6050 IMU** | 1 | 3-axis gyroscope and 3-axis accelerometer (I2C) | 3.3V – 5V | [MPU-6050 Datasheet (PDF)](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf) |

---

## Wiring Guide

**Important:** Make sure the negative (`-`) lead of your battery pack, the `GND` pin on the DRV8833 motor driver, and the `GND` pins on the Arduino Uno are all connected together (common ground). Do **not** power the DRV8833 `VM` pin from the Arduino's `5V` pin—run motor power directly from the battery pack.

### 1. DRV8833 Motor Driver & N20 Motors
The DRV8833 requires 4 PWM-capable pins on the Arduino Uno (marked with a `~` on the board) to control the speed and direction of both motors.

| DRV8833 Pin | Connects To | Notes |
| :--- | :--- | :--- |
| `VCC` / `VM` | Battery `+` (6V–9V) | Motor power supply |
| `GND` | Battery `-` and Arduino `GND` | Common ground |
| `EEP` / `SLP` | Arduino **Pin `4`** (or `5V`) | Driver enable pin (must be `HIGH` for the motors to run) |
| `AIN1` (IN1) | Arduino **Pin `~5`** | Left motor PWM input 1 |
| `AIN2` (IN2) | Arduino **Pin `~6`** | Left motor PWM input 2 |
| `BIN1` (IN3) | Arduino **Pin `~9`** | Right motor PWM input 1 |
| `BIN2` (IN4) | Arduino **Pin `~10`** | Right motor PWM input 2 |
| `AOUT1`, `AOUT2` | Left Motor terminals | Swap these two wires if the left wheel spins backward when commanded forward |
| `BOUT1`, `BOUT2` | Right Motor terminals | Swap these two wires if the right wheel spins backward when commanded forward |

### 2. Pololu QTR Reflectance Sensors (Line Detection)
Mount both sensors underneath the front-left and front-right corners of your chassis, roughly **3 mm to 6 mm** off the ground.

| Module | Sensor Pin | Arduino Uno Pin | Variable Name in Code |
| :--- | :--- | :--- | :--- |
| Both Modules | `VCC` / `5V` | `5V` | — |
| Both Modules | `GND` | `GND` | — |
| Front-Left QTR | `OUT1` | **`A0`** | `PIN_QTR_L1` (Left outer) |
| Front-Left QTR | `OUT2` | **`A1`** | `PIN_QTR_L2` (Left inner) |
| Front-Right QTR | `OUT1` | **`A2`** | `PIN_QTR_R1` (Right inner) |
| Front-Right QTR | `OUT2` | **`A3`** | `PIN_QTR_R2` (Right outer) |

### 3. HC-SR04 Ultrasonic Distance Sensor
Mount this on the front of the robot facing straight ahead.

| HC-SR04 Pin | Arduino Uno Pin | Notes |
| :--- | :--- | :--- |
| `VCC` | `5V` | 5V power |
| `Trig` | **Pin `7`** | `PIN_SONAR_TRIG` (Trigger input) |
| `Echo` | **Pin `8`** | `PIN_SONAR_ECHO` (Echo output) |
| `GND` | `GND` | Common ground |

### 4. MPU-6050 IMU (Gyroscope & Accelerometer)
Mount the IMU flat near the middle of the chassis. It connects over I2C.

| MPU-6050 Pin | Arduino Uno Pin | Notes |
| :--- | :--- | :--- |
| `VCC` | `5V` | Breakout board includes a voltage regulator |
| `GND` | `GND` | Common ground |
| `SDA` | **`A4`** (or `SDA`) | I2C Data |
| `SCL` | **`A5`** (or `SCL`) | I2C Clock |

---

## Software Setup & Libraries

### 1. Install the Arduino IDE
Download and install the [Arduino IDE](https://www.arduino.cc/en/software).

### 2. Install the Required Libraries
Before compiling [`starter_code/starter_code.ino`](starter_code/starter_code.ino), install these two libraries:

1. **Adafruit MPU6050**
   * Open the Arduino IDE and go to **Tools -> Manage Libraries...** (or click the Library Manager icon on the left).
   * Search for **`Adafruit MPU6050`** and click **Install**.
   * Click **Install All** when prompted to install its dependencies (`Adafruit Unified Sensor` and `Adafruit BusIO`).
   * Repository: [github.com/adafruit/Adafruit_MPU6050](https://github.com/adafruit/Adafruit_MPU6050)

2. **AlashMotorControlLite**
   * Search for **`AlashMotorControlLite`** in the Library Manager and click **Install**.
   * If it does not appear in your Library Manager, download the ZIP file from [github.com/Alash-electronics/AlashMotorControlLite](https://github.com/Alash-electronics/AlashMotorControlLite) (`Code` -> `Download ZIP`), then in the Arduino IDE go to **Sketch -> Include Library -> Add .ZIP Library...** and select the downloaded file.

---

## Programming & Calibration Guide

### Helper Functions in the Starter Code
Open [`starter_code/starter_code.ino`](starter_code/starter_code.ino). The following functions are already set up for you:

```cpp
// Set left and right motor speeds (-100 = full reverse, 0 = stop, 100 = full forward)
driveMotors(80, 80);    // Drive straight forward
driveMotors(-70, -70);  // Reverse straight back
driveMotors(60, -60);   // Spin clockwise (right) in place
driveMotors(-60, 60);   // Spin counter-clockwise (left) in place

// Active brake both wheels
stopMotors();

// Read one of the four line sensor channels (returns 0 on bright white to 1023 on dark black)
int reading = readLineSensor(PIN_QTR_L1);

// Read ultrasonic distance in cm (returns 999 if nothing is within ~85 cm)
int dist = readDistanceCm();
```

### Calibrating Your Robot Before a Match

1. **Verify Motor Directions:**
   Temporarily test `driveMotors(50, 50);`. Both wheels should spin forward. If one wheel goes backward, swap its two motor wires on the DRV8833 output terminals (`OUT1` and `OUT2`).
2. **Calibrate `BLACK_THRESHOLD`:**
   * Open **Tools -> Serial Monitor** (set baud rate to **`115200`**) and print out the values of `readLineSensor(PIN_QTR_L1)`.
   * Place the robot on the white part of the arena and check the reading (typically `100`–`300`).
   * Place the sensor over a black boundary ring and check the reading (typically `700`–`950`).
   * Set `BLACK_THRESHOLD` at the top of [`starter_code.ino`](starter_code/starter_code.ino) to a number halfway between your white and black readings (default is `650`).

### Ideas for Improving Your Code
The default `loop()` in [`starter_code.ino`](starter_code/starter_code.ino) only does the bare minimum. Once your robot works, consider adding:
* **Directional Edge Avoidance:** Check *which* side (`leftLine` vs `rightLine`) hit the black boundary line so you turn away from the edge instead of always turning right.
* **Ring Counting:** If the arena uses multiple concentric rings before the outer edge, count how many rings you cross so your robot knows how close it is to the boundary and can slow down near the edge.
* **Two-Stage Ramming:** Approach opponents at moderate speed (`70`–`80`) when they are far away, then switch to full speed (`100`) when `distanceCm < 15`.
* **Using the MPU-6050 IMU:**
  * Integrate `gyro.gyro.z` over time to turn by exact angles (e.g. `135°` or `180°`) instead of relying on `delay()`, which changes as your battery drains.
  * Use `gyro.gyro.z` to apply a small steering correction while driving forward so both N20 motors track straight when pushing an opponent.
  * Monitor `accel.acceleration.z` and `accel.acceleration.x`/`y` to detect if an opponent's wedge lifts your front wheels off the ground or rams you from the side.

---

## Troubleshooting

| Issue | Possible Cause | Solution |
| :--- | :--- | :--- |
| **Motors do not spin** | DRV8833 `EEP`/`SLP` pin is not `HIGH`, or battery pack is off | Check that `EEP`/`SLP` is wired to Pin `4` (or `5V`) and the battery is connected to `VM`. |
| **Robot spins in place when given `driveMotors(60, 60)`** | One motor's polarity is reversed | Swap the two motor wires on the side that is spinning backward. |
| **Arduino restarts when motors turn on** | Motors are connected to Arduino `5V` or batteries are flat | Connect DRV8833 `VM` directly to the battery `+`, not the Arduino `5V` pin. |
| **Ultrasonic sensor always returns `999`** | `Trig` and `Echo` wires are swapped | Verify `Trig` goes to Pin `7` and `Echo` goes to Pin `8`. |
| **Robot backs up constantly on white ground** | `BLACK_THRESHOLD` is too low or sensors are too high | Mount QTR sensors 3–6 mm above the floor and increase `BLACK_THRESHOLD`. |
