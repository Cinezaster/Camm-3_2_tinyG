# Camm-3_2_tinyG
Converting a Roland Camm-3 PNC-3000 CNC to a TinyG controller

## Spindle controller
An arduino reading the speed and adjust the phase of the power to the motor to hold target speed
* speed read
* spindle on/off read 
* target speed read
* pid control on motor

## Screen
Shows the status of the CNC {x,y,z,spindle speed, feedrate}

## Kill switch
A box with a big red kill switch, connected to reset.

## Auto height
Connect Z-min to GND.