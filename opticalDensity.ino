// foreground od measurements
float odVal = 0.0;
float odVal2 = 0.0;
float odVal3 = 0.0;
// for mean od calc.
float odtmp[3]; 
// background od measurements
float odValBg = 0.0;
float odVal2Bg = 0.0;
float odVal3Bg = 0.0;

double Vcc;

void odSetup()
{
  pinMode(irPin, OUTPUT);
  digitalWrite(irPin, LOW);  
  Serial.println("OD inited");
}

// reads OD value and writes in global state variable
void odUpdate()
{ 
  Vcc = readVcc() / 1000.0;
  analogWrite(ledPin, 0); 
  digitalWrite(pumpPin, LOW);
  delay(50);
  odValBg =  analogRead(odPin) / 1023.0 * Vcc;
  odVal2Bg =  analogRead(odPin2) / 1023.0 * Vcc;
  odVal3Bg =  analogRead(odPin3) / 1023.0 * Vcc;
  digitalWrite(irPin, HIGH);
  delay(500);
  for(int i=0; i<3; i++) {
    odtmp[i] =  analogRead(odPin) / 1023.0 * Vcc;
  }
  odVal = (odtmp[0] + odtmp[1] + odtmp[2]) / 3;
  odVal2 =  analogRead(odPin2) / 1023.0 * Vcc;
  odVal3 =  analogRead(odPin3) / 1023.0 * Vcc;
  digitalWrite(irPin, LOW);
  digitalWrite(pumpPin, airPumpState); // set pump back to pervious state (can be on/off)
  analogWrite(ledPin, lightBrightness);
  //  if(DEBUG)
  //  {
  //    Serial.print("The measured optical density of the liquid is: "); 
  //    Serial.println(odVal);
  //  }
}

float od()
{
  return odVal;
}

float od2()
{
  return odVal2;
}

float od3()
{
  return odVal3;
}

float odBg()
{
  return odValBg;
}

float od2Bg()
{
  return odVal2Bg;
}

float od3Bg()
{
  return odVal3Bg;
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

