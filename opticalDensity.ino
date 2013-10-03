
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

  Vcc = readVcc() / thousand;
  analogWrite(ledPin, 0); 
  delay(thousand); // wait for bubbling to settle

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
  delay(200);
  odValBg  = readSensor(odPin);
  odVal2Bg = readSensor(odPin2);
  // read foreground brightness and calculate OD
  digitalWrite(ledPins[emitterIdx], HIGH);
  delay(200);
  if(readReferenceValues)
  {
    od1RefValues[emitterIdx] = readSensor(odPin)  - odValBg;
    od2RefValues[emitterIdx] = readSensor(odPin2) - odVal2Bg;
  }
  else
  {
    od1Values[emitterIdx] = -log((readSensor(odPin)  - odValBg) / od1RefValues[emitterIdx]);
    od2Values[emitterIdx] = -log((readSensor(odPin2) - odVal2Bg) / od2RefValues[emitterIdx]);
  }
  digitalWrite(ledPins[emitterIdx], LOW);
}

// reads brightness from specified sensor. averages specified number of subsequent readings
float readSensor(int sensorPin)
{
  analogRead(sensorPin);  // discard first value from sensor
  sample = .0;
  for(int i=0; i<numReadingsAverage; i++)
    sample = sample + (analogRead(sensorPin) / valDiv * Vcc);
  return(sample / numReadingsAverage);
}

// taken from
// http://hacking.majenko.co.uk/making-accurate-adc-readings-on-arduino
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result;
}










