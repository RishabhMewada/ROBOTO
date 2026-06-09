# ROBOTO

<img width="1280" height="640" alt="RB-1 (2)" src="https://github.com/user-attachments/assets/74d9d698-4a90-4e6c-97ad-54e1d7a3c931" />

>  An 6 degree of freedom (dof) robot driven by Waveshare WP5320 servo and an esp32 and
  servo driver the robot is controlled from a glove consisting of an esp32 & BNO055 and a
  battery pack

![demo](docs/demo.gif)
![license](https://img.shields.io/badge/license-MIT-blue)

## Table of Contents
- [Overview](#overview)
- [Hardware](#hardware--wiring)
- [Software Architecture](#software-architecture)

## Overview
>'The why?'
 
**The sole purpose of this project is to help people build stuff and make them learn about robotics in comparatively affordable price and tinker around, which is easy to build and learn**

>The what?

**The project was build with one thing in mind, make it easy to connect and build (Like lego's !) , the project works on an ESP32 , and a PCA9685 servo driver to control the servo's and the servos's which are used in this project is Waveshare WP5320 *(it's MG996R servo in the project as it has similar diemensions to the WP5320 as i was not able to find the STEP file for it)* and BNO055 IMU to control the robot via hand gestures and a battery pack to power the glove side**

> The how?

**The project was Inspired from the S101 robot Repository and is built in fusion 360 with a minimilist Design Philosophy with a yet strong  build enough to handle its own weight and the objects it picks up without any bottleneck between the servo and it's built quality , and the entire robotic arm is build from ABS 3D printing material for a strong build with also some PLA in some parts of the robotic arm**

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
 
### Pin Map

## Software Architecture
### System Diagram
### Folder Structure
### Communication Protocol
 
## Usage & Demo
 Comming Sooon !!!
## License
