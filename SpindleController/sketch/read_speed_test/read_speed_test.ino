
#define AC_OUTPUT_PIN 5

volatile byte rpmcount;
unsigned int rpm;
unsigned long timeold;


void setup() 
{
	Serial.begin(115200);
	while (!Serial);

	pinMode(AC_OUTPUT_PIN, OUTPUT);
	digitalWrite(AC_OUTPUT_PIN, HIGH);

	attachInterrupt(1, spindle_speed_pulse, FALLING);

	rpmcount = 0;
   	rpm = 0;
   	timeold = 0;
}

void loop()
{
	detachInterrupt(1);
   //Note that this would be 60*1000/(millis() - timeold)*rpmcount if the interrupt
   //happened once per revolution instead of twice. Other multiples could be used
   //for multi-bladed propellers or fans
   rpm = (60/8)*1000/(millis() - timeold)*rpmcount;
   timeold = millis();
   rpmcount = 0;
   attachInterrupt(1, spindle_speed_pulse, FALLING);

   Serial.println(rpm);

   delay(200);
}

void spindle_speed_pulse()
{
	rpmcount++;
}