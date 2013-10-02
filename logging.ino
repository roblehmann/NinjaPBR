void loggingSetup()
{
// nothing to do for me here...
}

void loggingEvent()
{
  // send to computer
  Serial.print("DATA,");
  for(int i=0; i < numLeds; i++)
  {
    Serial.print(od1Values[i]);
    Serial.print(sep);
  }
  for(int i=0; i < numLeds; i++)
  {
    Serial.print(od2Values[i]);
    Serial.print(sep);
  }
  Serial.print(odValBg);
  Serial.print(sep);
  Serial.print(odVal2Bg);
  Serial.print(sep);
  Serial.print(temperatureMeasuredInLiquid());
  Serial.print(sep);
  Serial.print(dayPhase);
  Serial.print(sep);
  Serial.print(lightBrightness);
  Serial.print(sep);
  Serial.print(BIOREACTOR_MODE);
  Serial.print(sep);
  Serial.print(PHASE_DURATIONS[PHASE_MORNING]);
  Serial.print(sep);
  Serial.print(PHASE_DURATIONS[PHASE_DAY]);
  Serial.print(sep);
  Serial.print(PHASE_DURATIONS[PHASE_EVENING]);
  Serial.print(sep);
  Serial.print(PHASE_DURATIONS[PHASE_NIGHT]);
  Serial.print(sep);
  Serial.print(lightChangeStep);  
  Serial.print(sep);
  Serial.print(lightChangeStepLength);
  Serial.print(sep);
  Serial.print(minLightBrightness);
  Serial.print(sep);
  Serial.print(maxLightBrightness);
  Serial.print(sep);
  Serial.print(sensorSamplingTime);  
  Serial.println();
}









