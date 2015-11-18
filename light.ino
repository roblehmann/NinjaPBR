/*-------------------------
management of irradiation in
sample pods
-------------------------*/



/*-----------------------
setup function
-----------------------*/
void lightSetup()
{
  //set timer 2 (9,10) to 31.374 KHz
  TCCR2B = (TCCR2B & 0xF8) | 0x01;
  //set timer 1 (11,12) to 31.374 KHz
  TCCR1B = (TCCR1B & 0xF8) | 0x01;
  
  // initialize all light pins
  for(int i=0; i<3; i++)
    pinMode(lightPins[i], OUTPUT);
  // initially switch light off
  setMaxMinLight(MIN_LIGHT);
}

/*-----------------------
checks if reactor is in circadian mode, then updates brightness
-----------------------*/
void lightUpdate(int channel)
{
  switch (BIOREACTOR_MODE) {
  case BIOREACTOR_DYNAMIC_MODE:
    {
      // advance index
      lightProfileIdx[channel] += 1;
      // start from beginning if necessary
      // to allow for shorter light profiles than the full lightProfileLength, also return to first value if duration 0 is found
      if( (lightProfileIdx[channel] >= lightProfileLength) | (brightnessDuration[channel][lightProfileIdx[channel]] == 0) )
        lightProfileIdx[channel] = 0;
      // set panel brightness
      lightBrightness[channel] = brightnessValue[channel][lightProfileIdx[channel]];
      analogWrite(lightPins[channel], lightBrightness[channel]);
      // update light brightness after appropriate time
      if(channel == 0)
        lightChangeTimerID[channel] = t.after(long(brightnessDuration[channel][lightProfileIdx[channel]]) * 1000L, lightUpdate0); 
      else if(channel == 1)
        lightChangeTimerID[channel] = t.after(long(brightnessDuration[channel][lightProfileIdx[channel]]) * 1000L, lightUpdate1); 
      else
        lightChangeTimerID[channel] = t.after(long(brightnessDuration[channel][lightProfileIdx[channel]]) * 1000L, lightUpdate2);
      break;
    }
  case BIOREACTOR_LIGHT_MODE:
    {
      setMaxMinLight(MAX_LIGHT);
      break;
    }
  case BIOREACTOR_DARK_MODE | BIOREACTOR_ERROR_MODE:
    {
      setMaxMinLight(MIN_LIGHT);
      break;
    }
  default: 
    break;
  }
}

/*-----------------------
dirty hack to circumvent not being able to pass parameters via timer callback functions
-----------------------*/
void lightUpdate0()
{
  lightUpdate(0);
}
void lightUpdate1()
{
  lightUpdate(1);
}
void lightUpdate2()
{
  lightUpdate(2);
}

/*------------------
switch light on permanently 
------------------*/
void setMaxMinLight(int flag) 
{
  // most importantly, set brightness to min
  for (int i = 0; i < 3; i++) {
    if(flag == MIN_LIGHT)
      lightBrightness[i] = minLightBrightness[i];
    else
      lightBrightness[i] = maxLightBrightness[i];
      
    // write to pin, to make it effective immediately
    analogWrite(lightPins[i], lightBrightness[i]);

    // remove light change time (dynamic light mode) if set before
    if(lightChangeTimerID[i] != timerNotSet) 
    {
      t.stop(lightChangeTimerID[i]); 
      lightChangeTimerID[i] = timerNotSet;
    }
  }
}

/*-----------------------
dynamic light regulation setup 
-----------------------*/
void startDynamicLightTimers()
{
  for(int i=0; i<3; i++)
  {
    // make sure to have only one timer running
    if(lightChangeTimerID[i] != timerNotSet) {    // remove if set before
      t.stop(lightChangeTimerID[i]); 
      lightChangeTimerID[i] = timerNotSet;      // mark timer as not set
    }
    // set light profile pointer to end, so that calling advaning function it start from the beginning
    lightProfileIdx[i] = lightProfileLength;
    lightUpdate(i);
  }
}



