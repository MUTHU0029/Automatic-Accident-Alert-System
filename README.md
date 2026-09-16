# Automatic Accident Alert System

## Overview

An Arduino-based embedded IoT system that detects vehicle accidents
using accelerometer and gyroscope data and automatically sends the
vehicle's GPS location through SMS using a GSM module.

## Features

- Accident detection using MPU6050
- Accelerometer-based impact detection
- Gyroscope-based rotation detection
- 5-second alert cancellation window
- GPS location acquisition
- Emergency SMS using SIM800
- Google Maps location link
- Buzzer-based accident warning
- Accident alert cooldown mechanism

## Hardware

- Arduino Uno
- MPU6050
- SIM800 GSM Module
- GPS Module
- Buzzer
- Push Button

## Communication

- I2C - MPU6050
- UART/SoftwareSerial - GPS
- UART/SoftwareSerial - SIM800

## Pin Configuration

| Component | Arduino Pin |
|---|---|
| SIM800 TX | D8 |
| SIM800 RX | D7 |
| GPS TX | D4 |
| GPS RX | D3 |
| Buzzer | D6 |
| Cancel Button | D10 |

## Working

1. MPU6050 continuously measures acceleration and gyroscope values.
2. The system calculates acceleration and rotation magnitude.
3. If either value crosses the configured threshold, an accident
   condition is detected.
4. The buzzer is activated for 5 seconds.
5. The user can cancel the alert using the push button.
6. If there is no response, GPS coordinates are obtained.
7. SIM800 sends the location through SMS.
8. A cooldown period prevents repeated alerts.

## Libraries

- Adafruit MPU6050
- Adafruit Unified Sensor
- TinyGPS++
- SoftwareSerial
- Wire

## Future Improvements

- RTOS-based task management
- Firebase real-time tracking
- Better accident classification
- GSM network status monitoring
- GPS validity checking
- Emergency contact management
