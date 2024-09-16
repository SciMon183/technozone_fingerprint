# Technozone fingerprint lock door 

>This project wac created by person who need to providing acces to school's science club for all permitted members. In the future it will be expanded to work with backend server hosted on Raspberry PI whitch would contain logs providing information about each lock opening. This solusion can connect two or more finger lock door in science club. 

## Table of contents 
1. [Gneral info](#general-info) 
2. [Features](#features) 
3. [Pictures](#pictures) 
4. [Setup](#setup)   <!-- to:do in feature is do a good setup section -->
5. [Credits](#credits) 


## General info
Popular solution based on Arduino r3 providing fingerprint foor lock. For beginers is very good becous Arduino IDE just work with uno r3 without any special instalation etc.  


## Features
* Chage Arduino uno r3 with esp32 or 8266. 
* RGB LED signalling various program status. 
* Raspberry PI zero like server to connect two or more dors 


## Pictures <!-- do a good looking pictures if i actualy done -->
![First photo - this is a first prototype](./photo/first_proto.png)
> _TODO_

## Setup 
#### You will need: 
* Arduino IDE with working Arduino r3 board.
* Fingerprint sensor connectes through logic level converter. Fingerprint `rx` and `tx` connected to pin `3` and `2`
* Relay switch with connected electromagnetic lock to the `7` pin 
* Sttus diode connected to `3,3V` and `GND`

#### Use 
If you plug your board to the computer and upload the code all should start working. First you need to add some fingerprint. To do this you need to open `Serial Monitor` in Arduino IDE and chage the comunication speed to `9600`. <br>

1. If you need to add a fingerprint write in Serial Monitor `add` and do everything like program tell you. 
2. If you need to remove a finger print write in Serial Monitor `remove ID`.
3. If you need to see who has got a fingerprint or ID write in Serial Monitor `print`

All information you need to add some fingerprint has got a program so just listen what they talk to you in serial monitor

#### Schematic
> _TODO_


## Credits
Big thanks for: 

* [Adafruit fingerprint library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library)
* [Arduino SoftwareSerial library](https://docs.arduino.cc/learn/built-in-libraries/software-serial/)