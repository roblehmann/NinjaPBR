
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



// index indicating which wavelength's signal intensity or background is measured
// -1 - background, 0,..,4 index of emitter leds and hence index in data storage arrays
int nLambda = -1;

int nextWavelengthReadTimerID = -1;

// state variable indicating if air pump is working
boolean ancient_pump_state = HIGH;

//--------------------//
// function coordinating start of measuring background and signal values of different wavelength 
// across culture chambers 
//--------------------//
void odUpdateStart()
{
  // make sure first background and then different wavelengths are measured
  nLambda = -1;
  ancient_pump_state = airPumpState; // remember if pump was active before OD measurement
  stopAirPump(); //then deactivate
  analogWrite(ledPin, 0); // turn off light if active
  delay(100); // wait for bubbling to settle
  // start series of update steps for background intensity and individual wavelengths
  odUpdate();
}

//--------------------//
// putting all functions back into normal working mode after measuring OD
//--------------------//
void odUpdateStop()
{
  // put air pump back in original state
  if(ancient_pump_state == HIGH)
    startAirPump();
  analogWrite(ledPin, lightBrightness); 

  // set back wavelength state variable to background
  nLambda = -1;    
  if(readReferenceValues) // if reference value set was read, set back to normal mode now
  {
    readReferenceValues = false;    // normal OD mode
    sendReferenceValues();          // send reference values to interface
  }
  else  // send OD to computer
  {
    loggingEvent();
  }
  // check if turbidostat is active and dilution is necessary
  turbidityUpdate();
}
//--------------------//
// function reading sensors for one set wavelength (or background, as determined by state variable nLambda) from all avail. chambers
// in different culture chambers 
//--------------------//
void odUpdate()
{
  // ------------------------------------
  // reading process 
  // read current Vcc level for calculation
  Vcc = readVcc() / thousand;
  // read intensities from each chamber for current wavelength

  // switch on emitter if necessary (nLambda = measure background, stored in col 5)
  if(nLambda > -1)
    digitalWrite(ledPins[nLambda], HIGH);

  for(int iChamber=0; iChamber < numChambers; iChamber++)
  {
    // need to check if background value / reference value / OD is measured and di according calculation
    if(readReferenceValues)
    {
      if(nLambda == -1) // ref background values, no need to bg subtraction
      {
        refValues[iChamber][5]  = readSensor(odPin[iChamber]);
      } 
      else // store reference intensity, not OD
      {
        refValues[iChamber][nLambda] = abs(readSensor(odPin[iChamber])  - refValues[iChamber][5]); 
        Serial.print(nLambda); 
        Serial.print(" ref: "); 
        Serial.println(refValues[iChamber][nLambda]);
      }
    } 
    else
    {
      if(nLambda == -1) // background values, no need to bg subtraction
      {
        odValues[iChamber][5]  = readSensor(odPin[iChamber]);
      }
      else // normal OD values
      {
        odValues[iChamber][nLambda] = -log(abs(readSensor(odPin[iChamber])  - odValues[iChamber][5]) / refValues[iChamber][nLambda]); 
        Serial.print(nLambda); 
        Serial.print(" OD: "); 
        Serial.println(odValues[iChamber][nLambda]);
      }
    }
  }
  // switch off emitter again
  if(nLambda > -1)
    digitalWrite(ledPins[nLambda], LOW);
  // ------------------------------------

  // ------------------------------------
  // calling next wavelengths reading process
  if(nLambda >= (numLeds-1)) // done reading available wavelengths - finish OD reading
  {
    odUpdateStop();  // put back pump in orig mode and switch light on
  }
  else // read OD and go to next wavelength
  {
    nLambda += 1;
    // finally, call this function after waiting period for the next wavelength
    nextWavelengthReadTimerID = t.after(sensorReadDelay, odUpdate);
  }
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
long readVcc() 
{
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

//// array storing the resulting OD values for logging
//float od1Values[numChambers][numLeds] = {0};
//float od2Values[numChambers][numLeds] = {0};
//
////reference values for OD calculation
//// ir850, ir740, red, green, blue
//float od1RefValues[numChambers][numLeds] = {5};
//float od2RefValues[numChambers][numLeds] = {5};
//
//// background od measurements
//float odValBg[numChambers] = {0};
//float odVal2Bg[numChambers] = {0};
//
//// ------------------------------------------------------------ //
//// reads OD value and writes in global state variable
//void odUpdateOld()
//{ 
//  // remember if pump was active before OD measurement, then deactivate
//  boolean orig_pump_state = airPumpState; 
//  stopAirPump();
//  // turn of light if active
//  analogWrite(ledPin, 0); 
//  delay(100); // wait for bubbling to settle
//  // read current Vcc level for calculation
//  Vcc = readVcc() / thousand;
//
//  for(int i=numLeds-1; i >= 0; i--)
//    readOdSensorsOld(i);
//
//  // put air pump back in original state
//  if(orig_pump_state == HIGH)
//    startAirPump();
//  analogWrite(ledPin, lightBrightness);
//}
//
//// read OD values from the two sensors for a single emitter
//void readOdSensorsOld(int emitterIdx)
//{
//  // read background brightness
//  delay(sensorReadDelay);
//  for(int i=0; i<numChambers; i++)
//  {
//    odValBg[i]  = readSensor(odPin[i]);
//    odVal2Bg[i] = readSensor(odPin2[i]);
//  }
//  // read foreground brightness and calculate OD
//  digitalWrite(ledPins[emitterIdx], HIGH);
//  delay(sensorReadDelay);
//  if(readReferenceValues)
//  {
//    for(int i=0; i<numChambers; i++)
//    {
//      od1RefValues[i][emitterIdx] = abs(readSensor(odPin[i])  - odValBg[i]);
//      od2RefValues[i][emitterIdx] = abs(readSensor(odPin2[i]) - odVal2Bg[i]);
//    }
//  }
//  else
//  {
//    for(int i=0; i<numChambers; i++)
//    {
//      od1Values[i][emitterIdx] = -log(abs(readSensor(odPin[i])  - odValBg[i]) / od1RefValues[i][emitterIdx]);
//      od2Values[i][emitterIdx] = -log(abs(readSensor(odPin2[i]) - odVal2Bg[i]) / od2RefValues[i][emitterIdx]);
//    }
//  }
//  // switch off emitter again
//  digitalWrite(ledPins[emitterIdx], LOW);
//}



