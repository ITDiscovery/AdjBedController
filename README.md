# AdjBedController

This is a reverse engineering of an adjustable bed motor controller, because the cost of replacements on ebay is ridiculous:

![alt text](SealyEaseRemote-eBay.png?raw=true)

I spent quite a bit of time trying to figure out how to make a new remote, only to find out that it did not use bluetooth, but an nRF24L01 (2.4GHz) tranciever. Time to figure out how the relay circuit works.

Turning a motor on and off, or two motors one direction or another, is a really simple thing to accomplish with a microcontroller. I've used H-Bridges to solve this on small motors. Higher power and higher voltage motors aren't really that much of a different challenge, and the ciruit used in my bed controller is more comlicated than it needs to be only because what looks to be avoiding higher cost components. That's fine, designing a circuit to be mass produced has different challenges than a one off. The replacement design I have will favor spending a dollar more for a more robust solution that a mass produced design would avoid.

First let's break down the board that came with my bed:
![alt text](AdjBedRelayCrkt.JPG?raw=true)

It uses a standard (but pretty old and cheap) STM8S003 micro controller to drive a biased transistor that drives the base of an NPN transistor via a 3.3V GPIO with the help of R21 and R22. When the transistor T5 is on, current flows through the coil of the relay to ground, and the relay switches from 3-5 to 3-4, which turns the motor on in that direction. There are 3 more of these circuits to provide the two motors with two directions. 

![alt text](MotorDriverSchematic.jpg?raw=true)

Here's a better one, (but more expensive):
![alt text](BetterMotorDriverSchematic.png?raw=true)


https://circuitdigest.com/microcontroller-projects/arduino-dc-motor-speed-direction-control
