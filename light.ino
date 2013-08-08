void lightSetup()
{
  pinMode(ledPin, OUTPUT);
  // initially switch light off
  lightOff();
  //  Serial.println("inited light");
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
        Serial.println("ERROR:'PHASE_NONE' in circ.mode");
        BIOREACTOR_MODE = BIOREACTOR_ERROR_MODE;
        break;
      }
      break;
    }
  case BIOREACTOR_LIGHT_MODE:
    {
      lightOn();
      break;
    }
  case BIOREACTOR_DARK_MODE | BIOREACTOR_ERROR_MODE:
    {
      lightOff();
      break;
    }
//  default: // for every other reactor mode, the lightBrightness has been set before, just make sure that the value is written to pin
//    analogWrite(ledPin, lightBrightness);
//    break;
  }
}

// advances the day phase to the next
void dayPhaseUpdate()
{  
  if(BIOREACTOR_MODE == BIOREACTOR_CIRCADIAN_MODE)
  {
    // advance day phase by one
    dayPhase += 1; 
    if(dayPhase > PHASE_NIGHT) dayPhase = PHASE_MORNING; // if last phase of day (night), start new day (morning)
    // set new timer until next day phase change
    dayPhaseChangeTimerID = t.after(long(PHASE_DURATIONS[dayPhase]) * thousand, dayPhaseUpdate);
  }
}

//---------   switch light on permanently ---------//
void lightOff() 
{
  // most importantly, set brightness to min
  lightBrightness = minLightBrightness;
  // write to pin, to make it effective immediately
  analogWrite(ledPin, lightBrightness);
  if(dayPhaseChangeTimerID != timerNotSet) {
    t.stop(dayPhaseChangeTimerID);
    dayPhaseChangeTimerID = timerNotSet;
  }
  if(lightChangeTimerID != timerNotSet) {    // remove if set before
    t.stop(lightChangeTimerID); 
    lightChangeTimerID = timerNotSet;
  }
  dayPhase = PHASE_NONE;        // no day phases needed
}

//---------   switch light on permanently ---------//
void lightOn()
{
  // most importantly, set brightness to max
  lightBrightness = maxLightBrightness;
  // write to pin, to make it effective immediately
  analogWrite(ledPin, lightBrightness);
  if(dayPhaseChangeTimerID != timerNotSet) {
    t.stop(dayPhaseChangeTimerID);
    dayPhaseChangeTimerID = timerNotSet;
  }
  if(lightChangeTimerID != timerNotSet) {    // remove if set before
    t.stop(lightChangeTimerID); 
    lightChangeTimerID = timerNotSet;      // mark timer as not set
  }
  dayPhase = PHASE_NONE;        // no day phases needed
}

//---------  circadian light regulation setup --------------//
void startCircadianLightTimers() 
{
  if(lightChangeTimerID != timerNotSet) {    // remove if set before
    t.stop(lightChangeTimerID); 
    lightChangeTimerID = timerNotSet;      // mark timer as not set
  }
  lightChangeTimerID = t.every(long(lightChangeStepLength) * thousand, lightUpdate); // adjust light brightness
  dayPhase = PHASE_NIGHT;
  dayPhaseUpdate(); // advance day phase
}








