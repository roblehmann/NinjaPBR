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
    sensorReadTimerID = t.every(sensorSamplingTime * thousand,updateSensorLogValues);
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
