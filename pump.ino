void pumpSetup()
{
   // setup air pump
  pinMode(airValvePin, OUTPUT);
  digitalWrite(airValvePin, LOW);
  // setup medium pump
  pinMode(mediumPumpPin, OUTPUT);
  digitalWrite(mediumPumpPin, LOW);
}

//---------   air supply part ---------//
void startAirPump()
{
  // air by pump on/off control
  airPumpState = HIGH;
  // air by pinch valv control
  digitalWrite(airValvePin, LOW);
}

void stopAirPump() 
{
  // air by pump on/off control
  airPumpState = LOW;
  digitalWrite(airValvePin, HIGH);
}

//---------   medium supply part for turbidostat ---------//
// performs dilution
void doDilution(int duration)
{
  startMediumPump();
  t.after(duration * thousand, stopMediumPump);
}
// starts dilution 
void startMediumPump()
{
  digitalWrite(mediumPumpPin, HIGH);
}
// stops dilution
void stopMediumPump() 
{
  digitalWrite(mediumPumpPin, LOW);
}

// checks if turbidostat mode is activated. if so, checks if upper OD is
// exceeded, then starts dilution process. if already in dilution process, checks if
// OD is below lower threshold, then stops.
void turbidityUpdate()
{
  // TODO: could combine starting and continuing condition. but for clarity...
  if(!inTurbidostat) return; // turbidostat not active
  // TS active, and dilution not started and upper OD at 750nm (sensor2) is exceeded -> start dilution process
  if(!inDilution & od2Values[1][1] > upperOdThr) 
  {
    inDilution = true;
    doDilution(dilutionDuration);
  }
  // TS active, dilution started and lower OD at 750nm (sensor2) is exceeded -> continue dilution process
  else if(inDilution & od2Values[1][1] > lowerOdThr)
  {
    doDilution(dilutionDuration);
  }
  // TS active, dilution started and OD is below lower thr -> stop dilution process
  else if(inDilution & od2Values[1][1] <= lowerOdThr)
  {
    inDilution = false;
  }
}

