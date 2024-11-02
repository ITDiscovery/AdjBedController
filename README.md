# AdjBedController

This is a reverse engineering of an adjustable bed motor controller, because the cost of replacements on ebay is ridiculous:

![alt text](SealyEaseRemote-eBay.png?raw=true)

I spent quite a bit of time trying to figure out how to make a new remote, only to find out that it did not use bluetooth, but an nRF24L01 (2.4GHz) tranciever. Time to figure out how the relay circuit works.

Turning a motor on and off, or two motors one direction or another, is a really simple thing to accomplish with a microcontroller. I've used H-Bridges to solve this on small motors. Higher power and higher voltage motors aren't really that much of a different challenge, and the ciruit used in my bed controller is more comlicated than it needs to be only because what looks to be avoiding higher cost components. That's fine, designing a circuit to be mass produced has different challenges than a one off. The replacement design I have will favor spending a dollar more for a more robust solution that a mass produced design would avoid.

First let's break down the board that came with my bed:
![alt text](AdjBedRelayCrkt.JPG?raw=true)

It uses a standard (but pretty old and cheap) STM8S003 micro controller to drive a biased transistor that drives the base of an NPN transistor via a 3.3V GPIO with the help of R21 and R22. When the transistor T5 is on, current flows through the coil of the relay to ground, and the relay switches from 3-4 to 3-5, turning the motor on in that direction. There are 3 more of these circuits to provide the two motors with two directions. Two blocking diodes D19 and D20 allow the companion relay K4 to apply voltage to the motor in the opposite direction.  

![alt text](MotorDriverSchematic.jpg?raw=true)

There's a catch though: These motors have a switch in them so that when the motor has reached maximum or minimum, the motor turns off. The microcontroller must keep an eye on that so that it can switch off the relay when the motor goes to open circuit. That's pretty easy, as you can see here:
![alt text](AdjBedFeedBack.JPG?raw=true)  

This connects the NC part of the relay (3-4) via dropping resistor R41 and damping capacitor C24 back to the the microcontroller analog input. That's placed so that it doesn't really matter which relay of the pair (K3 and K4 in our case) is on, voltage will drop as long as the motor is running. You can see the companion circuit for the other motor here at R40/C23. 

Since the controller board isn't damaged, I simply used those circuits in place and replaced the STM8S003 with an ESP-32. To make sure the STM8S003 didn't interfere, I eventually removed it completely. Initially though I removed the resistor at R21, and put a 2k resistor on the GPIO of the ESP-32. This board at least has a voltage regulator, so it was pretty easy to pick up power and ground from the board for the ESP-32.
![alt text](AdjBedMotorController.jpg?raw=true)   

Here's a better one, (but more expensive):
![alt text](BetterMotorDriverSchematic.png?raw=true)




https://circuitdigest.com/microcontroller-projects/arduino-dc-motor-speed-direction-control
