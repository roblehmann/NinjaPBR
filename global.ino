void updateSensorLogValues()
{
  // update sensor data
  temperatureUpdate();
  odUpdate();
  // send to computer
  loggingEvent();
}

// starts timer to sample sensors and send them to the computer
// if a measuring timer was already active, dont change anything
void startSensorReadTimer() 
{
  if(sensorReadTimerID == timerNotSet) 
  {
    sensorReadTimerID = t.every(sensorSamplingTime * thousand,updateSensorLogValues);
  } 
//  else 
//  {
//    if(DEBUG)
//      Serial.println("sensorReadTimer already exists, returning...");
//  }
}

void stopSensorReadTimer() 
{
  if(sensorReadTimerID != timerNotSet) 
  {
    t.stop(sensorReadTimerID);
    sensorReadTimerID = timerNotSet;
  }
//  else
//  {
//    if(DEBUG)
//      Serial.println("sensorReadTimer not active, returning...");
//  }
}



