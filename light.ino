void lightSetup()
{
  // initially switch light off
  lightOff();
  Serial.println("The LED panel has been initialized.");
}

// should be called every 'lightChangeStepLength' millisecs.
// checks if reactor is in circadian mode, then updates brightness
void lightUpdate()
{  

  switch (BIOREACTOR_MODE) {
  case BIOREACTOR_CIRCADIAN_MODE:
    {
      switch (dayPhase) {
      case PHASE_MORNING:
        //increase brightness until max
        if(lightBrightness < maxLightBrightness)
//          lightBrightness += lightChangeStep;
          lightBrightness = min(lightBrightness+lightChangeStep, maxLightBrightness);
        analogWrite(ledPin, lightBrightness);
        break;
      case PHASE_DAY:
        //do nothing
        lightBrightness = maxLightBrightness;
        analogWrite(ledPin, lightBrightness);
        break;
      case PHASE_EVENING:
        //decrease brightness until off
        if(lightBrightness > minLightBrightness)
//          lightBrightness -= lightChangeStep;
          lightBrightness = max(lightBrightness-lightChangeStep, minLightBrightness);
        analogWrite(ledPin, lightBrightness);
        break;
      case PHASE_NIGHT:
        //do nothing
        lightBrightness = minLightBrightness;
        analogWrite(ledPin, lightBrightness);
        break;
      case PHASE_NONE:
        //ERROR: should not be here when reactor in circadian mode
        Serial.println("ERROR: day phase can not be 'PHASE_NONE' when reactor is in circadian mode, aborting...");
        BIOREACTOR_MODE = BIOREACTOR_ERROR_MODE;
        break;
      }
    }
  default: // for every other reactor mode, the lightBrightness has been set before, just make sure that the value is written to pin
    analogWrite(ledPin, lightBrightness);
    break;
  }
}

// advances the day phase to the next
void dayPhaseUpdate()
{  
  if(BIOREACTOR_MODE == BIOREACTOR_CIRCADIAN_MODE)
  {    
    switch (dayPhase) {
    case PHASE_MORNING:
      dayPhase = PHASE_DAY;
      t.after(PHASE_DAY_DURATION * 1000, dayPhaseUpdate);
      break;
    case PHASE_DAY:
      dayPhase = PHASE_EVENING;
      t.after(PHASE_EVENING_DURATION * 1000, dayPhaseUpdate);
      break;
    case PHASE_EVENING:
      dayPhase = PHASE_NIGHT;
      t.after(PHASE_NIGHT_DURATION * 1000, dayPhaseUpdate);
      break;
    case PHASE_NIGHT:
      dayPhase = PHASE_MORNING;
      t.after(PHASE_MORNING_DURATION * 1000, dayPhaseUpdate);
      break;
    case PHASE_NONE:
      //ERROR: should not be here when reactor in circadian mode (i.e. the dayPhaseTimer is registered)
      //      Serial.println("ERROR: day phase can not be 'PHASE_NONE' when reactor is in circadian mode, starting with PHASE_MORNING...");
      dayPhase = PHASE_MORNING;
      t.after(PHASE_MORNING_DURATION * 1000, dayPhaseUpdate);
      break;
    }
//    if(DEBUG) Serial.println(dayPhase);
  }
}

//---------   switch light on permanently ---------//
void lightOff() 
{
  // most importantly, set brightness to min
  lightBrightness = minLightBrightness;
  // write to pin, to make it effective immediately
  analogWrite(ledPin, lightBrightness);

  if(lightChangeTimerID > -1)    // remove if set before
    t.stop(lightChangeTimerID); 
  lightChangeTimerID = -1;      // mark timers as not set
  dayPhase = PHASE_NONE;        // no day phases needed

}

//---------   switch light on permanently ---------//
void lightOn()
{
  // most importantly, set brightness to max
  lightBrightness = maxLightBrightness;
  // write to pin, to make it effective immediately
  analogWrite(ledPin, lightBrightness);

  if(lightChangeTimerID > -1)    // remove if set before
    t.stop(lightChangeTimerID); 
  lightChangeTimerID = -1;      // mark timer as not set
  dayPhase = PHASE_NONE;        // no day phases needed

}

//---------  circadian light regulation setup --------------//
void startCircadianLightTimers() 
{
  
  lightChangeTimerID = t.every(lightChangeStepLength * 1000, lightUpdate); // adjust light brightness
  dayPhase = PHASE_NIGHT;
  dayPhaseUpdate(); // advance day phase
}
