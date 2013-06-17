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
  if(sensorReadTimerID == -1) 
  {
    sensorReadTimerID = t.every(sensorSamplingTime * 1000,updateSensorLogValues);
  } 
//  else 
//  {
//    if(DEBUG)
//      Serial.println("sensorReadTimer already exists, returning...");
//  }
}

void stopSensorReadTimer() 
{
  if(sensorReadTimerID != -1) 
  {
    t.stop(sensorReadTimerID);
    sensorReadTimerID = -1;
  }
//  else
//  {
//    if(DEBUG)
//      Serial.println("sensorReadTimer not active, returning...");
//  }
}



