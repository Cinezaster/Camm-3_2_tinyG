#include <PID_v1.h>
#include <TimerOne.h>

#define VERBOSE 1 //In verbose mode I talk alot. I talking might screw up some timings

#define SPINDLE_ON_OFF_IN 6 //I listen to the controller boards spindle on/off output
#define FREQ 50
#define AC_PIN 5	// Output to Opto Triac

bool spindle_on = false;
bool run_motor = false;
bool pwm_measured = false;
volatile int pwm_start_time;
volatile int pwm_middle_time;
volatile int pwm_high_time;
volatile int pwm_total_time;
volatile int pwm_duty;
int spindle_speed_read_count = 0;
volatile int spindle_speed_start_measurment_time;
unsigned long int period = 1000000 / (2 * FREQ);//The Timerone PWM period in uS
double wait = 3276700000;
unsigned int onTime = 0;	// the calculated time the triac is conducting
unsigned int offTime = period-onTime;	//the time to idle low on the AC_PIN

//Define Variables we'll be connecting to
double target_speed, current_speed, Output;

//Specify the links and initial tuning parameters
PID myPID(&current_speed, &Output, &target_speed,2,5,1, DIRECT);

void Init_input () {
	#ifdef VERBOSE
		Serial.println("Init_input");
		Serial.println("setting pinMode");
	#endif
	pinMode(SPINDLE_ON_OFF_IN, INPUT);
	pinMode(AC_PIN, OUTPUT);		// Set the Triac pin as output
}

void Initiate_motor_start_sequence () {
	target_speed = Get_target_speed();
	Start_spindle_speed_measuring();
	attachInterrupt(0, zero_cross_detect, RISING);
}

int Get_target_speed () {
	attachInterrupt(4, pwm_start_reading, RISING);
	int start_measuring = millis();
	while(!pwm_measured){
	    if (start_measuring + 2000 > millis()) {
	    	detachInterrupt(4);
	    	pwm_duty = 100;
	    	pwm_measured = true;
	    }
	}
	return map(pwm_duty, 0, 100, 2000, 7000);
}

void pwm_start_reading () {
	attachInterrupt(4, pwm_middle_reading, FALLING);
	pwm_start_time = micros();
}

void pwm_middle_reading () {
	attachInterrupt(4, pwm_end_reading, RISING);
	int now = micros();
	pwm_high_time = now - pwm_start_time;
}

void pwm_end_reading () {
	detachInterrupt(4);
	int now = micros();
	pwm_total_time = now - pwm_start_time;
	pwm_duty = (pwm_total_time/pwm_high_time)*100;
	pwm_measured = true;
}

void Start_spindle_speed_measuring() {
	attachInterrupt(1, spindle_speed_pulse, FALLING);
	spindle_speed_read_count = 0;
	spindle_speed_start_measurment_time = micros();
}

void spindle_speed_pulse() {
	spindle_speed_read_count++;
}

void Shut_down_motor () {
	detachInterrupt(1);
	detachInterrupt(0);
	Timer1.stop();
}

void zero_cross_detect()	// function to be fired at the zero crossing.  This function
{				
    Timer1.restart();	// or attaches nowIsTheTime to fire at the right time.
    if (offTime<=100)			//if off time is very small
    {
        digitalWrite(AC_PIN, HIGH);	//stay on all the time
    }
    else if (offTime>=8000) {		//if offTime is large
        digitalWrite(AC_PIN, LOW);	//just stay off all the time
    }
    else	//otherwise we want the motor at some middle setting
    {
        Timer1.attachInterrupt(spindle_pulse_out,offTime);
    }
}


void spindle_pulse_out ()
{
	digitalWrite(AC_PIN,HIGH);
    wait = sqrt(wait);		//delay wont work in an interrupt.
    if (!wait)                      // this takes 80uS or so on a 16Mhz proc
    {
        wait = 3276700000;
    }
    digitalWrite(AC_PIN,LOW);
}

void setup () {
	Serial.begin(115200);
	while (!Serial) ;

	//turn the PID on
  	myPID.SetMode(AUTOMATIC);

  	Timer1.initialize(period);
}

void loop () {
	if (!spindle_on && digitalRead(SPINDLE_ON_OFF_IN)) {
		spindle_on++;
		Initiate_motor_start_sequence();
	} else if (spindle_on && !digitalRead(SPINDLE_ON_OFF_IN)){
		spindle_on = false;
		Shut_down_motor();
	}

	if (run_motor) {
		// if spindle is not spinning
		if ((micros() - spindle_speed_start_measurment_time) > 1000) {
			current_speed = 0;
		}
		// after 8 pulses the spindle made one tour
		// this methode adds a small error
		if(spindle_speed_read_count >= 8) {
			spindle_speed_read_count = 0;
			int spindle_one_rotation_time = micros() - spindle_speed_start_measurment_time;
			spindle_speed_start_measurment_time = micros();
			current_speed = spindle_one_rotation_time/60000000;
			#ifdef VERBOSE
				Serial.print("Current speed: ");
				Serial.println(current_speed);
			#endif
		}
		myPID.Compute();
		#ifdef VERBOSE
			Serial.print("Output: ");
			Serial.println(Output);
		#endif

		onTime = map(Output, 0, 100, 0, period);	// re scale the value from hex to uSec 
        offTime = period - onTime;			// off is the inverse of on, yay!

	}
}