#define MainPeriod 100
long previousMillis = 0; // will store last time of the cycle end
volatile unsigned long duration=0; // accumulates pulse width
volatile unsigned int pulsecount=0;
volatile unsigned long previousMicros=0;

void setup() {
  Serial.begin(19200);
   // while the serial stream is not open, do nothing:
   while (!Serial) ;
  // put your setup code here, to run once:
  attachInterrupt(0, time, RISING);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MainPeriod) 
  {
    previousMillis = currentMillis;   
    // need to bufferize to avoid glitches
    unsigned long _duration = duration;
    unsigned long _pulsecount = pulsecount;
    duration = 0; // clear counters
    pulsecount = 0;
    float Freq = 1e6 / float(_duration);
    Freq *= _pulsecount; // calculate F
    // output time and frequency data to RS232
    Serial.print(currentMillis);
    Serial.print(" "); // separator!
    Serial.print(Freq);
    Serial.print(" "); 
    Serial.print(_pulsecount);
    Serial.print(" ");
    Serial.println(_duration);
  }
}
 
void time () {
  unsigned long currentMicros = micros();
  duration += currentMicros - previousMicros;
  previousMicros = currentMicros;
  pulsecount++;
}