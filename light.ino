
void lightSetup()
{
  pinMode(ledPin, OUTPUT);
  // initially switch light off
  lightOff();
}

// checks if reactor is in circadian mode, then updates brightness
void lightUpdate()
{  
  switch (BIOREACTOR_MODE) {
  case BIOREACTOR_DYNAMIC_MODE:
    {
      // advance index
      lightProfileIdx += 1;
      // start from beginning if necessary
      // to allow for shorter light profiles than the full lightProfileLength, also return to first value if duration 0 is found
      if(lightProfileIdx >= lightProfileLength | brightnessDuration[lightProfileIdx] == 0)
        lightProfileIdx = 0;
      // set panel brightness
      lightBrightness = brightnessValue[lightProfileIdx];
      analogWrite(ledPin, lightBrightness);
      // update light brightness after appropriate time
      lightChangeTimerID = t.after(long(brightnessDuration[lightProfileIdx]) * thousand, lightUpdate); 
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
  default: 
    break;
  }
}

//---------   switch light on permanently ---------//
void lightOff() 
{
  // most importantly, set brightness to min
  lightBrightness = minLightBrightness;
  // write to pin, to make it effective immediately
  analogWrite(ledPin, lightBrightness);
  if(lightChangeTimerID != timerNotSet) {    // remove if set before
    t.stop(lightChangeTimerID); 
    lightChangeTimerID = timerNotSet;
  }
}

//---------   switch light on permanently ---------//
void lightOn()
{
  // most importantly, set brightness to max
  lightBrightness = maxLightBrightness;
  // write to pin, to make it effective immediately
  analogWrite(ledPin, lightBrightness);
  if(lightChangeTimerID != timerNotSet) {    // remove if set before
    t.stop(lightChangeTimerID); 
    lightChangeTimerID = timerNotSet;      // mark timer as not set
  }
}

//---------  dynamic light regulation setup --------------//
void startDynamicLightTimers()
{
  // make sure to have only one timer running
  if(lightChangeTimerID != timerNotSet) {    // remove if set before
    t.stop(lightChangeTimerID); 
    lightChangeTimerID = timerNotSet;      // mark timer as not set
  }
  // set light profile pointer to end, so that calling advaning function it start from the beginning
  lightProfileIdx = lightProfileLength;
  lightUpdate();
}



