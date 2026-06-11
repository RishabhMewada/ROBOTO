# ROBOTO

<img width="1280" height="640" alt="RB-1 (2)" src="https://github.com/user-attachments/assets/74d9d698-4a90-4e6c-97ad-54e1d7a3c931" />

>  An 6 degree of freedom (dof) robot driven by Waveshare WP5320 servo and an esp32 and
  servo driver the robot is controlled from a glove consisting of an esp32 & BNO055 and a
  battery pack

![demo](docs/demo.gif)
![license](https://img.shields.io/badge/license-MIT-blue)

## Table of Contents
- [Overview](#overview)
- [Hardware](#Hardware-&-Wiring-Diagram)
- [Software Architecture](#software-architecture)

## Overview
>'The why?'
 
**The sole purpose of this project is to help people build stuff and make them learn about robotics in comparatively affordable price and tinker around, in a much easier way to build and learn** and to be honest i also wanted to learn about robotics and improve my skills and build stuff which is hard to build !!

>The what?

**The project was build with one thing in mind, make it easy to connect and build (Like lego's !) , the project works on an ESP32 , and a PCA9685 servo driver to control the servo's and the servos's which are used in this project is Waveshare WP5320 *(it's MG996R servo in the project as it has similar diemensions to the WP5320 as i was not able to find the STEP file for it)* and BNO055 IMU to control the robot via hand gestures and a battery pack to power the glove side**

> The how?

**The project was Inspired from the S101 robot Repository and is built in fusion 360 with a minimilist Design Philosophy with a yet strong  build enough to handle its own weight and the objects it picks up without any bottleneck between the servo and it's built quality , and the entire robotic arm is build from ABS 3D printing material for a strong build with also some PLA in some parts of the robotic arm**

<p align="center">
<img width="1920" height="722" alt="ROBOTO-PARTS_2026-May-29_05-07-04PM-000_CustomizedView9396335264" src="https://github.com/user-attachments/assets/7fc38e2b-9d12-4ac9-9ae1-a7e20343c35a" />
</p>

## Hardware & Wiring Diagram

**Use the Wiring diagram as the Pin map for connecting everything and use the link above the diagram to get the exact pins!**

>Hardware (Robotic Arm)

1. ESP32 
2. PCA9685 Servo driver
3. Waveshare WP5320 Servo 180 Degree

>Wiring Schematic (Robotic Arm)

 [Click here to view the Schematic!](https://app.cirkitdesigner.com/project/19c6ec55-5556-4408-a9a6-c3da8fffa251)
 
<img width="1280" height="640" alt="RB-1 (3)" src="https://github.com/user-attachments/assets/bd2dfecf-67b3-429d-8272-0743bf161713" />

----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

>*Hardware (Glove)*

-Battery pack
1. 18650 Battery
2. TP4056 battery charger
3. MT608 boost converter
4. rocker switch
  
-Motion capture module
1. ESP32
2. BNO055 IMU 9-DOF
3. Flex Sensor
   
>Wiring Schematic (Glove)

 [Click here to view the Schematic!](https://app.cirkitdesigner.com/project/269f68dc-bc2f-4ff6-8df4-b6a70eb212e9)

<img width="1280" height="640" alt="RB-1" src="https://github.com/user-attachments/assets/1e14d738-25a4-4bad-bc1e-f999358d6251" />

-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

### Bill of Materials

> Total project cost: **~$173.31 USD**

## Electronics

| # | Component | Purpose | Qty | Cost (USD) | Distributor |
|---|-----------|---------|:---:|:----------:|-------------|
| 1 | [ESP32 WROOM-32](https://robu.in/product/esp-wroom-32-wifi-bluetooth-networking-smart-component-development-board/) | Main control unit (glove + arm) | 2 | $8.70 | Robu.in |
| 2 | [BNO055 IMU](https://robu.in/product/df-robot-febno055-intelligent-9-axis-sensor-rmion-bno055-intelligent-9-axis-sensor-breakout/) | 9-axis IMU for arm motion tracking | 1 | $19.87 | Robu.in |
| 3 | [PCA9685](https://robocraze.com/products/pca9685-16-channel-servo-motor-driversoldered?variant=40193677721753&country=IN&currency=INR) | 16-channel 12-bit servo driver | 1 | $2.40 | Robocraze.com |
| 4 | [WP5320 Servo](https://hubtronics.in/wp5320) | Arm joints + gripper (×6) | 6 | $82.30 | Hubtronics.in |
| 5 | [Flex Sensor 2.2"](https://robocraze.com/products/flex-sensor-2-2?variant=40192405504153&country=IN&currency=INR) | Gripper trigger on glove | 1 | $2.03 | Robocraze.com |

## Power System

| # | Component | Purpose | Qty | Cost (USD) | Distributor |
|---|-----------|---------|:---:|:----------:|-------------|
| 6 | [Samsung 18650 Li-Ion Battery](https://robu.in/product/samsung-18650-30q-li-ion-battery/) | Main power source for glove | 1 | $6.06 | Robu.in |
| 7 | [18650 Battery Holder](https://robu.in/product/1-x-18650-cell-box/) | Battery housing | 1 | $6.06 | Robu.in |
| 8 | [TP4056 Charger Module](https://robocraze.com/products/tp4056-battery-charger-c-type-module-with-protection-1?variant=46170796261600&country=IN&currency=INR) | Li-Ion battery charger with protection | 1 | $0.15 | Robocraze.com |
| 9 | [MT3608 Boost Converter](https://robu.in/product/mt3608-2a-max-dc-dc-step-up-power-module-booster-power-module/) | Steps up battery voltage for ESP32 | 1 | $0.35 | Robocraze.com |
| 10 | [Rocker Switch](https://robu.in/product/6a-250v-ac-spst-on-off-rocker-switch/) | Power on/off | 1 | $0.17 | Robu.in |

## Mechanical & Build

| # | Component | Purpose | Qty | Cost (USD) | Distributor |
|---|-----------|---------|:---:|:----------:|-------------|
| 11 | [3D Printing Service](https://robu.in/product/3d-printing-service1/) | Arm structure parts | 16 | $34.96 | Robu.in |
| 12 | [Screws & Thread Inserts](https://onlyscrews.in/a/cart?cart-token=2tY5s2&link-token=10JKFq) | Assembly hardware | 1 | $4.10 | onlyscrews.in |

## Glove Assembly

| # | Component | Purpose | Qty | Cost (USD) | Distributor |
|---|-----------|---------|:---:|:----------:|-------------|
| 13 | [Fingerless Compression Glove](https://www.amazon.in/MOMISY-Copper-Infused-Fingerless-Compression-Tendonitis/dp/B09HV4BFLP) | Base for motion sensor pack | 1 | $5.02 | Amazon.in |
| 14 | [Velcro Straps](https://www.amazon.in/Glimzo-Velcro-Tape-Buckles-Management/dp/B0GX9J13YC) | Holds battery pack on hand | 1 | $0.72 | Amazon.in |
| 15 | [Jumper Wires (F-F 10cm)](https://robu.in/product/10cm-female-female-breadboard-jumper-dupont-2-54mm-1p-1p-cable-40-pcs/) | Module connections | 1 | $0.42 | Robu.in |

---

> **Note:** Prices are in USD and may vary by region. All Indian suppliers ship domestically.
> Servo power supply (5–6 V / 3 A BEC) is required separately for the arm and is not included above. 

 
## Software Architecture

- The system is split across two independent ESP32 microcontrollers communicating wirelessly in real time. The glove controller reads motion and finger data and then packages it into a compact struct, and broadcasts it over to the ESP-NOW at 50 Hz.
  
- The arm controller receives each packet and maps the orientation values to servo angles and it applies exponential smoothing, and drives six WP5320 servos through a PCA9685 PWM driver. No router, no Bluetooth pairing, no internet — the two boards talk directly peer-to-peer over 2.4 GHz with sub-5 ms latency
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
## System Diagram

<p align="center">
 <img width="1895" height="2587" alt="Group 1" src="https://github.com/user-attachments/assets/34868a1c-8a43-4853-8362-e5542ff2e3aa" />
</p>

-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

## Folder Structure

```robotic-glove-arm/
│
├── firmware/
│   ├── arm_esp32/
│   │   └── arm_esp32.ino        # Arm controller — ESP-NOW RX, PCA9685, servo mapping
│   ├── glove_esp32/
│   │   └── glove_esp32.ino      # Glove controller — BNO055, flex sensor, ESP-NOW TX
│   └── shared/
│       └── config.h             # GlovePacket struct, pin defines, shared constants
│
├── hardware/
│   ├── schematics/
│   │   ├── glove_wiring.png     # Exported wiring diagram
│   │   └── arm_wiring.png       # Exported wiring diagram
│   ├── cad/
│   │   └── arm_assembly.step    # Full CAD assembly of the robot
|   |   └── 3D print Files       #  3D models for 3D printing
│   └── BOM.csv                  # Part, qty, supplier, price, link
│
├── Images/
│   ├── banner.png               # README header image (1500×500 px)
|   └── robtic arm & glove       # CAD images of the robot and the glove
│
├── README.md
├── CHANGELOG.md
├── LICENSE
└── .gitignore
```

## Communication Protocol

- The two boards communicate exclusively over ESP-NOW — a connectionless, peer-to-peer Wi-Fi protocol built into the ESP32. No pairing process, no router required, and no TCP overhead. Once the arm's MAC address is registered as a peer on the glove, packets fire unidirectionally from glove to arm at 50 Hz with approximately 2–4 ms over-the-air latency.
Packet structure — GlovePacket (30 bytes)

```typedef struct __attribute__((packed)) {
    float    roll;        // degrees, offset-corrected  (-180 … +180)
    float    pitch;       // degrees, offset-corrected  (-90  … +90)
    float    yaw;         // degrees, offset-corrected  (0    … 360)
    float    quatW;       // quaternion W component
    float    quatX;       // quaternion X component
    float    quatY;       // quaternion Y component
    float    quatZ;       // quaternion Z component
    uint8_t  gripClose;   // 1 = close gripper, 0 = open
    uint8_t  calMode;     // 1 = calibration reset signal, 0 = normal
    int16_t  flexRaw;     // raw ADC value (0–4095) for diagnostics
} GlovePacket;            // total: 30 bytes
```
- Transmission
  
| Property | Value |
|---|---|
| Protocol | ESP-NOW (IEEE 802.11 vendor action frames) |
| Direction | Glove → Arm (unidirectional) |
| Frequency | 50 Hz (every 20 ms) |
| Packet size | 30 bytes |
| Latency | ~2–4 ms |
| Range | ~200 m open air, ~50 m indoors |
| Encryption | None (add `encrypt: true` in peer config to enable) |
| Timeout | 500 ms — arm returns to HOME if no packet received |


- Calibration packets — 

**when the user triggers calibration on the glove (BOOT button held 3 s), the glove captures its neutral orientation and flex thresholds, saves them to NVS, then sends one packet with calMode = 1. The arm receives this and moves to HOME position, resetting its own smoothing state. Normal calMode = 0 packets resume immediately after**

- Connection loss handling — 

**the arm tracks the timestamp of the last received packet. If 500 ms passes with no packet (glove powered off, out of range, or obstruction), all servo targets are set to HOME and the status LED blinks at 300 ms intervals until signal resumes**

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
## Build it !!

### firmware

***for the step by step guide on how to install the firmware on the ESP32 and download the  Prerequisites required for the installation of the firmware on ESP32 and make it work please go to the guide pdf***
it would be lenghty to write it here
                                                                                 
```
Firmware-installation.pdf

``` 


## Wirring steps 

please follow the schematic diagram in the readme or visit the links here, and connect everything via female to female jumper wires or solder them wires onto the components , i recommend jumper wires it keeps the project components resuable!

1. [Click here to view the Schematic!](https://app.cirkitdesigner.com/project/19c6ec55-5556-4408-a9a6-c3da8fffa251) for the wiring of the robotic arm

2.  [Click here to view the Schematic!](https://app.cirkitdesigner.com/project/269f68dc-bc2f-4ff6-8df4-b6a70eb212e9) for the wiring of glove

please not that the wiring in glove will require the components to be soldered via wires, which should be connected by following the schematics for both the glove and the Arm

## Robotic Arm build & glove build

>i need to build first but here is some basic guide

step 1. add the base servo motor inside the first servo adapter box SB1 and then slide/clip it inside the base servo motor bracket and then attach the servo arm joint onto the motor , attach the BR/BASE rotation arm on the arm joint and add brass inserts in the BR and arm joint when attached via a solder into the four holes and add screws into those holes use 4x15mm brass inserts and 4x15MM screws

Step 2.

 
## Usage & Demo
 Comming Sooon !!! (in build stage)
 
## License
**MIT LICENSE**


## Zine
 <p align="center">
 <img width="647" height="993" alt="RB-1" src="https://github.com/user-attachments/assets/3ffca880-9faf-489d-8363-32a4f99b4ce2" />
</p>
