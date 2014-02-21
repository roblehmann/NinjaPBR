// index indicating which wavelength's signal intensity or background is measured
// -1 - background, 0,..,4 index of emitter leds and hence index in data storage arrays
int nLambda = -1;

// state variable indicating if air pump is working
boolean ancient_pump_state = HIGH;



void updateSensorLogValues()
{
  // update sensor data
  temperatureUpdate();
  odUpdateStart();
}

// starts timer to sample sensors and send them to the computer
// if a measuring timer was already active, dont change anything
void startSensorReadTimer() 
{
  if(sensorReadTimerID == timerNotSet) 
  {
    sensorReadTimerID = t.every(sensorSamplingTime * 1000L,updateSensorLogValues);
  } 
}

void stopSensorReadTimer() 
{
  if(sensorReadTimerID != timerNotSet) 
  {
    t.stop(sensorReadTimerID);
    sensorReadTimerID = timerNotSet;
  }
}

//-------------------------------------------------//
//-------------------------------------------------//
// function coordinating start of measuring signal/background values of different wavelength 
// across culture chambers 
void odUpdateStart()
{
  // make sure first background and then different wavelengths are measured
  nLambda = -1;
  ancient_pump_state = airPumpState; // remember if pump was active before OD measurement
  stopAirPump(); //then deactivate
  analogWrite(ledPin, 0); // turn off light if active
  delay(50); // wait for bubbling to settle
  // start series of update steps for background intensity and individual wavelengths
  odUpdate();
}

//-------------------------------------------------//
//-------------------------------------------------//
// putting all functions back into normal working mode after measuring OD;
// calls logging for signal and reference values upon update completion. 
// sets back referenceValue measurement state variable to default
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
