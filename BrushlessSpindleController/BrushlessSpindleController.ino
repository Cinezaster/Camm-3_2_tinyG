/* BRUSHLESS SPINDLE SPEED CONTROLLER
/*
/* This speed controller is build 
/* to control 
/* an ESC brushless motor
/* It adds an 128x64 Oled screen see the rpm
/* And lets you adjust the speed with an rotary encoder 


!!!!!!IMPORTANT !!!!!!
This script will only work with an arduino Leonardo due to interupts

*/

/* By Toon Nelissen aka Cinezaster */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <Bounce2.h>
#include <Encoder.h>
#include <Servo.h>
#include <RunningMedian.h>

#define ROTARYENC1 0
#define ROTARYENC2 1
#define RPMPULSEPIN 3
#define PUSHPIN 4
#define SPINDLESETPIN 5
// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

volatile int state = 0;
volatile int menuState = 0;
int knoppieSpeed;
double setSpeed, ESCInput, rpm;
boolean motor = false;
boolean spindelON = false;
volatile int rpmcount;
float timeold;
volatile double speedR = 1;

RunningMedian rpmSamples = RunningMedian(30);

Bounce pushbutton = Bounce(); // Debounce the pushbutton of the rotary encoder

Encoder knoppie(ROTARYENC1,ROTARYENC2); // rotary encoder libary

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS); // init of the oled screen

Servo ESC; // ESC servo object

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/* Util function to write the setspeed to the eeprom */
void EEPROMWritelong(int address, long value)
{
	//Decomposition from a long to 4 bytes by using bitshift.
	//One = Most significant -> Four = Least significant byte
	byte four = (value & 0xFF);
	byte three = ((value >> 8) & 0xFF);
	byte two = ((value >> 16) & 0xFF);
	byte one = ((value >> 24) & 0xFF);

	//Write the 4 bytes into the eeprom memory.
	EEPROM.write(address, four);
	EEPROM.write(address + 1, three);
	EEPROM.write(address + 2, two);
	EEPROM.write(address + 3, one);
}

/* Util function to read the setspeed from the eeprom */
long EEPROMReadlong(long address)
{
	//Read the 4 bytes from the eeprom memory.
	long four = EEPROM.read(address);
	long three = EEPROM.read(address + 1);
	long two = EEPROM.read(address + 2);
	long one = EEPROM.read(address + 3);

	//Return the recomposed long by using bitshift.
	return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

/* Display's the startup screen */
void display_start () 
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(43,16);
	display.println("Spindle");
	display.setCursor(32,27);
	display.println("Controller");
	display.setCursor(50,54);
	display.println("by T");
	display.display();
}

/* Display's the setspeed screen */
void display_setSpeed ()
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(4,16);
	display.println("Set speed:");
	display.setCursor(64,16);
	display.println((int)setSpeed);
	display.setCursor(4,30);
	display.println("RPM:");
	display.setCursor(64,30);
	display.println("/");
	display.display();
}

/* Display's the menu active speed screen */
void display_menuSetSpeed ()
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(5,16);
	display.println(">  Speed");
	
	display.setCursor(5,30);
	display.println("   Test spindle");

	display.display();
}

/* Display's the menu active test spindle screen */
void display_menuTest ()
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(5,16);
	display.println("   Speed");
	
	display.setCursor(5,30);
	display.println(">  Test spindle");
	
	display.display();
}

/* Display's setspeed for adjusting */
void display_speedDial ()
{
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(20,16);
	display.println((int)setSpeed);
	
	display.display();
}

/* Display's main screen while spindle is running */
void display_rpm ()
{
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(WHITE);
	display.setCursor(4,16);
	display.println("Set speed:");
	display.setCursor(64,16);
	display.println((int)setSpeed);
	display.setCursor(4,30);
	display.println("RPM:");
	display.setCursor(64,30);
	display.println((int)rpm);
	display.display();
}

/* Interrupt function */
void spindle_speed_pulse()
{
	rpmcount++;
}

/* Calculate the rpm */
void spindle_rpm() 
{
   detachInterrupt(1);
   float pastTime = (micros() - timeold)*1.00; // eleapse milli seconds
   float rotation = rpmcount; // One rotation == 8pulses multiply by 10000 to add precision 
   float div = rotation/pastTime; // ration divided by elapsedtime multiplyed by 10000 for precision
   float tpm = 60000000.00*div; // 6 = 60000/10000 = 1minute
   rpm = tpm/8.00; // I have 8 black patches on my encoder wheel
   timeold = micros();
   rpmcount = 0;
   attachInterrupt(1, spindle_speed_pulse, FALLING);
}

/* Initialize the ESC else it won't startup the first time */
void InitializeESC ()
{
	ESC.attach(5,1100,1900);
	ESC.writeMicroseconds(1100);
	delay(4000);
	ESC.writeMicroseconds(1150);
	delay(2000);
	ESC.detach();
}

/* Guess the pwm to start the motor */
void guesstimateESCInput ()
{
	long tspd = map(setSpeed,700,11000,10,45);
	ESCInput = map(tspd,0,100,1100,1900);
	ESC.writeMicroseconds(ESCInput * speedR);
}

/* Start sequence for the motor */
void startMotor ()
{
	motor = true;
	attachInterrupt(1, spindle_speed_pulse, FALLING);
	ESC.attach(5,1100,1900);
	ESC.writeMicroseconds(1100);
	timeold = micros();
	display_rpm();
	delay(1000);
	guesstimateESCInput();
	delay(2000);
}

/* Stop sequence for the motor */
void stopMotor ()
{
	motor = false;
	detachInterrupt(1);
	ESC.detach();
}

void adjustMotor () 
{
	spindle_rpm();
	rpmSamples.add(rpm);
	if (rpmSamples.getCount() == 30) {
		double med = rpmSamples.getAverage(6);
		if (med > setSpeed + 500) {
			speedR += 0.02;
		} else if (med < setSpeed - 500) {
			speedR -= 0.02;
		}
		rpmSamples.clear();
	}
	ESC.writeMicroseconds(ESCInput* speedR);
}

void setup () 
{
	Serial.begin(115200);
	//while (!Serial) ;

	pinMode(PUSHPIN, INPUT_PULLUP);

	pinMode(SPINDLESETPIN, INPUT);

	pushbutton.attach(PUSHPIN);
	pushbutton.interval(70);


	display.begin(SSD1306_SWITCHCAPVCC);

	display.setTextSize(1);
	display.setTextColor(WHITE);

	display_start();

	InitializeESC();

	setSpeed = EEPROMReadlong(0);

}

void loop ()
{
	switch (state) {
		case 0:				// Waiting screen
			display_setSpeed();
			break;
		case 1:				// menu SetSpeed selected
			display_menuSetSpeed();
			if((abs(knoppie.read())/3) % 2  == 1) 
			{
				state = 3;
				delay(100);
			}
			break;
		case 2:				// Dail in the speed
			display_speedDial();
			{
				int tempKnoppie = knoppie.read();
				if (knoppieSpeed != tempKnoppie) {
					if (tempKnoppie > 6 && tempKnoppie < 110) {
						knoppieSpeed = tempKnoppie;
						setSpeed = knoppieSpeed * 100;
					} 
					else
					{
						knoppie.write(knoppieSpeed);
					} 
				}
			}
			break;
		case 3:				// menu test selected
			display_menuTest();
			if ((abs(knoppie.read())/3) % 2 == 0)
			{
				state = 1;
				delay(100);
			} 
			break;
		case 4:				// motor on because testing it
			display_rpm();
			break;
		case 10:			// motor on because spindlePin is high
			display_rpm();
			break;
	}

	if(pushbutton.update() && pushbutton.read() == LOW){
		switch (state) {
			case 0:			// Go to menu
				state++;
				menuState = 0;
				knoppie.write(0);
				break;
			case 1:			// Go to set speed
				state++;
				knoppieSpeed = setSpeed/100;
				knoppie.write(knoppieSpeed);
				break;
			case 2:			// Set the speed and go back
				EEPROMWritelong(0,(long)setSpeed);
				if(spindelON){			// if spindlePin is high go back to state 10
				    state = 10;
				} else {
				    state = 0;
				}
				break;
			case 3:			// Start the motor for testing
				state++;
				startMotor();
				break;
			case 4:			// Stop the motor after testing
				state = 0;
				stopMotor();
				break;
			case 10:		// Go to setSpeed to adjust the speed
				state = 2;
				break;
		}
	}

	if(motor){
		if (rpmcount > 12) {
			adjustMotor();
		}
	}

	Serial.println(digitalRead(SPINDLESETPIN));

	/*if(!spindelON && digitalRead(SPINDLESETPIN))
	{
		spindelON = true;
		state = 10;
	    startMotor();
	} else if(spindelON && !digitalRead(SPINDLESETPIN)) 
	{
	    spindelON = false;
	    state = 0;
	    stopMotor();
	}*/
}