# AdjBedController

This is a reverse engineering of an adjustable bed motor controller, because the cost of replacements on ebay is ridiculous:

![alt text](SealyEaseRemote-eBay.png?raw=true)

I spent quite a bit of time trying to figure out how to make a new remote, only to find out that it did not use bluetooth, but an nRF24L01 (2.4GHz) tranciever. Time to figure out how the relay circuit works.

Turning a motor on and off, or two motors one direction or another, is a really simple thing to accomplish with a microcontroller. I've used H-Bridges to solve this on small motors. Higher power and higher voltage motors aren't really that much of a different challenge, and the ciruit used in my bed controller is more comlicated than it needs to be only because what looks to be avoiding higher cost components. That's fine, designing a circuit to be mass produced has different challenges than a one off. The replacement design I have will favor spending a dollar more for a more robust solution that a mass produced design would avoid.

First let's break down the board that came with my bed:
![alt text](AdjBedRelayCrkt.JPG?raw=true)

It uses a standard (but pretty old and cheap) STM8S003 micro controller via a 3.3V GPIO to drive the base of an NPN transistor (see R21 and R22). When the transistor T5 is on, current flows through the coil of the relay to ground, and the relay switches from 3-4 to 3-5, turning the motor on in that direction. There are 3 more of these circuits to provide the two motors with two directions. The relays are wired similar to what you would see with light switches ganged together.

![alt text](MotorDriverSchematic.jpg?raw=true)

There's a catch though: These motors have a switch in them so that when the motor has reached maximum or minimum, the motor turns off. The microcontroller must keep an eye on that so that it can switch off the relay when the motor goes to open circuit. That's pretty easy, as you can see here:
![alt text](AdjBedFeedBack.JPG?raw=true)

This connects the NC part of the relay (3-4) via a shunt resistor and a dropping resistor R41 and damping capacitor C24 back to the the microcontroller analog input. That's placed so that it doesn't really matter which relay of the pair (K3 and K4 in our case) is on, voltage will drop as long as the motor is running. You can see the companion circuit for the other motor here at R40/C23. 

Since the controller board isn't damaged, I simply used those circuits in place and replaced the STM8S003 with an ESP-32. To make sure the STM8S003 didn't interfere, I eventually removed it completely. Initially though I removed the resistor at R21, and put a 2k resistor on the GPIO of the ESP-32. This board at least has a voltage regulator, so it was pretty easy to pick up power and ground from the board for the ESP-32. I'm still having a weird problem where K3 turns on periodically for no reason. 
![alt text](AdjBedMotorController.jpg?raw=true) 

Turn the motor on with the ESP-32 and monitoring the output is pretty easy, and doing this via a subroutine is the best way to go:

```
void MotorOn(int relaypin, int motorpin, int runtm) {
    digitalWrite(relaypin, HIGH);
    int j=0;
    delay(100);
    while ((j<=runtm) && (analogRead(motorpin) >  MotorOnThresh)) {
      j++;
      delay(100);
    }
    digitalWrite(relaypin,LOW);
    //Add delay for multiple MotorOn commands
    delay(400);
}
```
It requires a little experimentation to understand what the threshold is for when the motor is on versus off. I measure that input every 100mSec and send the output LOW if the motor hits the endstop.

Here's the web page for this application:
![alt text](MotorOn-WebPage.jpg?raw=true) 

## Your controller board is damaged!

I just lost my remote (never mind that it didn't have any memory settings), so I just hacked the controller board, but this is a pretty easy circuit to design, and there are much more robust examples to turn on two motors in two directions. I found a good one here:

https://circuitdigest.com/microcontroller-projects/arduino-dc-motor-speed-direction-control

My design moves the speed controlling MOSFET to the input of both relays primarily because the software should never run both motors at the same time. The MOSFET circuit is optional, it just seems safer to me to allow the microcontroller to shut down the power if it sees a safety issue (like overheating). The opto-isolator I combined into a single package, as 4 are pretty close to the same price as one. That is a standard (but more expensive) solution that protects the ESP-32 if the switching transistor ever fails by shorting out.

Here's a simulation of how the a typical MotorOn happens:

Before the Motor Starts:  \
![alt text](MotorOn-Start.jpg?raw=true)   \
The Motor is running forward:  \
![alt text](MotorOn-RunFwd.jpg?raw=true)  \
The Motor is running backward:  \
![alt text](MotorOn-RunBck.jpg?raw=true)  \
What happens if the motor shuts off during MotorOn:  \
![alt text](MotorOn-MotorStopped.jpg?raw=true)  \
Nothing happens if you turn on both relays at the same time (but don't do this):  \
![alt text](MotorOn-BothRelays.jpg?raw=true)  \

I'm working on a board as a replacement for the entirety of the controller board above using the relay mechanism. The power input is the 30VDC adapter
that comes with these devices typically, and switches this voltage over to motors. Version 1 used a simple 7805 or a Hi-Link isolated mini switching supply, which due to the 30VDC input, didn't last long. I then tried an LM317, which worked....but got really hot (for the same reason). Learning my lesson that stepping down such a large voltage drop is better via a buck converter, Version 2 (which fixes a bunch of other dumb mistakes) uses a LM2596S-5.0 which is SMD mount (along with it's companion coil), but hopefully is hand solderable. I want this board to be easy to assemble, and may convert it to an all SMD design so that it can be fully manufactured by JCLPCB.

To do for version 3:

- Remove the line from Pin 5 to Ground.
- Enlarge the Schottky diode pads
- Widen 29V and other traces in buck converter area.
- Add 8 pin din for manual bypass controller
- Troubleshooting/assembly section. Specifically not to connection power supply until the 5V is verified, and that if the voltage is off, check for a good connection on the 56uH coil.

Proper H-Bridge wiring:

Relay K1:
- Common (COM): Connected to the red wire of the output.
- Normally Open (NO): Connected to Ground (GND).
- Normally Closed (NC): Connected to the +29V Rail.

Relay K2
- Common (COM): Connected to the black wire of the output.
- Normally Open (NO): Connected to the +29V Rail.
- Normally Closed (NC): Connected to Ground (GND).

With this wiring, to make the motor run in one direction, you would activate Relay K1 to connect one motor terminal to 29V, and leave Relay K2 un-energized to connect the other motor terminal to 29V. To reverse the direction, you would activate Relay 2 and leave Relay 1 un-energized.

You must also ensure that the control logic in your ESP32 never activates both relays in a pair at the same time. If you do, you will create a short circuit across the motor's terminals.


