/* 	Test input pulse of AC optocoupler and output of the SSR
	The sketch uses a sinus wave sweep through different timings for phase control
	Connect pin 3 to bidirectional optoCoupler output
*/
#include <TimerOne.h>

#define INTERRUPT_INPUT_PIN 3
#define AC_OUTPUT_PIN 5
#define FREQ 50

unsigned long int period = 1000000 / (2 * FREQ);
int offTime;
int x = 0;
const float pi = 3.14159;
double wait = 3276700000;
volatile int spindle_speed_read_count = 0;
volatile long spindle_speed_start_measurment_time;
volatile long spindle_one_rotation_time = 0;
int trigger = 0;

void setup()
{
	Serial.begin(115200);
	while (!Serial);

	pinMode(AC_OUTPUT_PIN, OUTPUT);
	
	attachInterrupt(0, zero_cross_detect, RISING);
	attachInterrupt(1, spindle_speed_pulse, FALLING);

	Timer1.initialize(period);
	Timer1.attachInterrupt(trigger_AC_pin);
    Timer1.disablePwm(10);

    spindle_speed_start_measurment_time = micros();
}

void loop()
{
	x++;
	float r = x*(pi/180); 				// change degrees to radians
	int y = (sin(r)+1)*100; 			//change range from -1/1 to 0/200
	offTime = map(y,0,200,180,6800);	//map range 0/200 to 200/9600
	Serial.print('offtime: ');
	Serial.println(offTime);									// total time between phase = 10 000
	if(x == 360){						// don't have to do this
	    x = 0;  						// but do it anyway
	}

	int current_speed = spindle_one_rotation_time/60000000;

	Serial.print('speed: ');
	Serial.print(current_speed);
	Serial.println(' rpm');

	delay(100); // interrupts
}

// there is no correction for the 200uS offset of the zero crossing
void zero_cross_detect()	// function to be fired at the zero crossing. 
{				
    digitalWrite(AC_OUTPUT_PIN,LOW);
    Timer1.setPeriod(offTime);
    Timer1.restart();
    trigger=0;
}

void trigger_AC_pin () {
	if(trigger == 0){
	    trigger++;
	} else {
		digitalWrite(AC_OUTPUT_PIN,HIGH);
	    Timer1.stop();
	}
}

void spindle_speed_pulse () {
	spindle_speed_read_count++;
	if(spindle_speed_read_count == 8)
	{
		spindle_speed_read_count = 0;
		spindle_one_rotation_time = micros() - spindle_speed_start_measurment_time;
		spindle_speed_start_measurment_time = micros();
	}
}