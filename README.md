## Centipede Arcade1Up Arduino Interface
### Goal of this project
First generation Arcade1Up uses proprietary circuit boards to interface with the game's circuit board through a 40 pin IDC cable. The goal of this project is to create an interface for any PC that can be hooked directly to their proprietary circuit board via the IDC cable and interpret the output as a joystick or mouse.

I chose the Arduino Leonardo for it's ability to emulate USB devices and the ease of creating shields with it's form factor. 

This is an update for the direct connection to the raspberry pi I used to use. Python proved too slow for my purposes and I was always worried about stealing cpu cycles that could of been better used for game performance. Moving this to a microprocessor fixed everything.

### IDC 40 Pinout
| Pin | Function |
|--|--|
| 1 | P1 Start |
| 2 | Uart Pin 2 |
| 3 | P2 Start |
| 4 | Uart pin 1 |
| 5 | A Button |
| 6 | B Button |
| 7 | C Button |
| 13 | UART 5V |
| 14 | UART GND |
| 18 | GND |
| 35 | Pwr Switch |
| 37 | Vol A(up) |
| 38 | Vol B(down) |
| 39 | Speaker |
| 40 | Speaker |

The UART on this is 5v, so we can directly connect it to our Arduino Leonardo. Every button needs it's own channel as well. 

### UART encoding
I used a logic analyzer to decode what was going on via the serial communication. Looking at the rate of pulses I found out it was 115200 speed. Counting the pulses based on that speed I found out it was 7n1.

The game board does not poll the controller board, the trackball just sends the data as you move the ball. It sends 4 bytes of data for every move like this:

    7F XX YY ZZ
XX and YY are the X and Y axis of the trackball and ZZ is in use when a spinner is installed(select cabinets). The values can be 7E(-1), 00(0), 01(1).


### Putting it together
Knowing how the UART works and what pins go where, I prototyped my device. Since I didnt need to convert voltages, it was simple to just make the traces go directly to the IDC 40 connector.

![View of Circuit Board Design](https://github.com/Christopher-R-Perkins/A1Up-Centipede-PI-Arduino/blob/main/IMAGES/CentipedeShield.png?raw=true)

Since I got the circuit working and programmed a solution, all I needed to do was fabricate a board and solder it. 

I decided to keep my automatic swapping by communicating via the serial port emulated by the USB. 

First we gotta make our device readable, so that we can send messages via BASH. Add this line to your /etc/rc.local, so that we can use it later in the runcommand-onstart/onend.sh scripts.
 `chmod o+rw /dev/ttyACM0`
You should now be able to put in the runncommand scripts and be good.

You'll find the fabrication files and the source code to this in this repository.
