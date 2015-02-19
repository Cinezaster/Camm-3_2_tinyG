#AC-Motor Speedcontroller

## Features
* Read ON/OFF signal from CNC Controller
* Read PWM out from CNC Controller
* Measures speed of the spindle
* Using PID control to set and hold speed
* Zero crossing detection to set phase control on AC line to motor

## Board
![Board](/SpindleController/board/board.png)
* H11AA1S AC opto coupler to detect the zero point crossing
* SDV2415R random triggered Solid State Relay
* R1 & R2 22K Ohm 2Watt resistors

## Arduino Leonardo pins
* Zero Crossing input --> Pin 3 (interrupt 0)
* Speed puls input --> Pin 2 (interrupt 1)
* Frequencie read input --> Pin 7 (interrupt 4)
* Spindle ON/OFF input --> Pin 6
* RX CNC board --> Pin 1 (TX)
* TX CNC board --> Pin 0 (RX)
* Phase puls output --> Pin 5
* Oled MOSI --> Pin SPI MOSI (Pin 14)
* Oled CLK --> Pin SPI CLK (Pin 15)
* Oled Reset --> Pin 8
* Oled Dc --> Pin 9
* Oled CS --> Pin 10
