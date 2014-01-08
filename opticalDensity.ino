
double Vcc = .0;
float  sample = .0;

void odSetup()
{
  pinMode(ir850Pin, OUTPUT);
  digitalWrite(ir850Pin, LOW);    
  pinMode(ir740Pin, OUTPUT);
  digitalWrite(ir740Pin, LOW);
  pinMode(redPin, OUTPUT);
  digitalWrite(redPin, LOW);    
  pinMode(greenPin, OUTPUT);
  digitalWrite(greenPin, LOW);    
  pinMode(bluePin, OUTPUT);
  digitalWrite(bluePin, LOW);    
}

// reads OD value and writes in global state variable
void odUpdate()
{ 
  // remember if pump was active before OD measurement, then deactivate
  boolean orig_pump_state = airPumpState; 
  stopAirPump();
  // turn of light if active
  analogWrite(ledPin, 0); 
  delay(400); // wait for bubbling to settle
  // read current Vcc level for calculation
  Vcc = readVcc() / thousand;
  
  for(int i=numLeds-1; i >= 0; i--)
    readOdSensors(i);

  // put air pump back in original state
  if(orig_pump_state == HIGH)
    startAirPump();
  analogWrite(ledPin, lightBrightness);
}

// read OD values from the two sensors for a single emitter
void readOdSensors(int emitterIdx)
{
  // read background brightness
  odValBg  = readSensor(odPin);
  odVal2Bg = readSensor(odPin2);
  // read foreground brightness and calculate OD
  digitalWrite(ledPins[emitterIdx], HIGH);
  delay(150);
  if(readReferenceValues)
  {
    od1RefValues[emitterIdx] = abs(readSensor(odPin)  - odValBg);
    od2RefValues[emitterIdx] = abs(readSensor(odPin2) - odVal2Bg);
  }
  else
  {
    od1Values[emitterIdx] = -log(abs(readSensor(odPin)  - odValBg) / od1RefValues[emitterIdx]);
    od2Values[emitterIdx] = -log(abs(readSensor(odPin2) - odVal2Bg) / od2RefValues[emitterIdx]);
  }
  digitalWrite(ledPins[emitterIdx], LOW);
}

// reads brightness from specified sensor. averages specified number of subsequent readings
float readSensor(int sensorPin)
{
  analogRead(sensorPin);  // discard first value from sensor
  sample = .0;
  for(int i=0; i<numReadingsAverage; i++)
    sample = sample + (analogRead(sensorPin) / 1023.0 * Vcc);
  return(sample / numReadingsAverage);
}

// taken from https://code.google.com/p/tinkerit/wiki/SecretVoltmeter
//
//const long scaleConst = 1156.300 * 1000 ; // internalRef * 1023 * 1000;
const long scaleConst = 1125300L;
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
#else
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif 

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH 
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = scaleConst / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return (long)result; // Vcc in millivolts
}
