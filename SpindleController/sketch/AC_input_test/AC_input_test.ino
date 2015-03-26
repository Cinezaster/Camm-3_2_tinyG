/* Test input pulse of AC optocoupler
	Connect pin 3 to bidirectional optoCoupler output
*/
#define INTERRUPTPIN 3

long prevTime;
int triggerCount = 0;

void setup()
{
	Serial.begin(115200);
	while (!Serial);
	
	attachInterrupt(0, zero_cross_detect, RISING);
}

void loop()
{
	if(triggerCount > 99){
	    triggerCount = 0;
	    Serial.print("triggerd :");
	    long timing = micros() - prevTime;
	    Serial.println(timing);
	    prevTime = micros();
	}
}

// there is no correction for the 200uS offset of the zero crossing
void zero_cross_detect()	// function to be fired at the zero crossing. 
{				
    triggerCount++;
}