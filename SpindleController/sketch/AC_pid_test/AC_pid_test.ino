/* 	Test input pulse of AC optocoupler and output of the SSR
	The sketch uses a sinus wave sweep through different timings for phase control
	Connect pin 3 to bidirectional optoCoupler output
*/
#include <TimerOne.h>
#include <PID_v1.h>

#define INTERRUPT_INPUT_PIN 3
#define AC_OUTPUT_PIN 5
#define FREQ 50
#define MAX_SPEED 7000

unsigned long int period = 1000000 / (2 * FREQ);
int offTime;
int x = 0;
const float pi = 3.14159;
double wait = 3276700000;
volatile int spindle_speed_read_count = 0;
volatile long spindle_pulse_start_measurment_time;
volatile long spindle_pulse_time = 0;
int trigger = 0;

double target_speed, current_speed, Output;

PID myPID(&current_speed, &Output, &target_speed,2,5,1, DIRECT);


void setup()
{
	Serial.begin(115200);
	while (!Serial);

	pinMode(AC_OUTPUT_PIN, OUTPUT);
	
	attachInterrupt(0, zero_cross_detect, RISING);
	attachInterrupt(1, spindle_speed_pulse, FALLING);
	spindle_pulse_start_measurment_time = micros();

	Timer1.initialize(period);
	Timer1.attachInterrupt(trigger_AC_pin);
    Timer1.disablePwm(10);

    myPID.SetOutputLimits(0, MAX_SPEED);
	//turn the PID on
  	myPID.SetMode(AUTOMATIC);

    
}

void loop()
{
	target_speed = 2200;
	Serial.print(target_speed);
	Serial.print(" ");

	current_speed = 8 * spindle_pulse_time/60000000;
	myPID.Compute();

	Serial.print(target_speed);
	Serial.print(", ");

        Serial.print(spindle_pulse_time);
	Serial.print(", ");

	long onTime = map(Output, 0, MAX_SPEED, 180, 6800);	// re scale the value speed to uSec 
	offTime = period - onTime;			// off is the inverse of on, yay!
}

// there is no correction for the 200uS offset of the zero crossing
void zero_cross_detect()	// function to be fired at the zero crossing. 
{				
    digitalWrite(AC_OUTPUT_PIN,LOW);
    Timer1.setPeriod(offTime);
    Timer1.restart();
    trigger=0;
}

void trigger_AC_pin () 
{
	if(trigger == 0){
	    trigger++;
	} else {
		digitalWrite(AC_OUTPUT_PIN,HIGH);
	    Timer1.stop();
	}
}

void spindle_speed_pulse () 
{
	spindle_pulse_time = micros() - spindle_pulse_start_measurment_time;
	spindle_pulse_start_measurment_time = micros();
}