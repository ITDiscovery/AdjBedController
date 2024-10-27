# AdjBedController

This is a reverse engineering of an adjustable bed motor controller, because the cost of replacements on ebay is ridiculous:

![alt text](SealyEaseRemote-eBay.png?raw=true)

Turning a motor on and off, or two motors one direction or another, is a really simple thing to accomplish with a microcontroller. I've used H-Bridges to solve this on small motors. Higher power and higher voltage motors aren't really that much of a different challenge, and the ciruit used in my bed controller is more comlicated than it needs to be only because what looks to be avoiding higher cost components. That's fine, designing a circuit to be mass produced has different challenges than a one off. The replacement design I have will favor spending a dollar more for a more robust solution that a mass produced design would avoid.

First let's break down the board that came with my bed:

It uses a standard (but pretty old and cheap) micro controller to drive a biased transistor that controls 

![alt text](MotorDriverSchematic.png?raw=true)

Here's a better one, (but more expensive):
![alt text](BetterMotorDriverSchematic.png?raw=true)
