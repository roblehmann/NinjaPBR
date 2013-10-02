void pumpSetup()
{
  airValve.attach(airValvePin);
  // setup air pump
  airPumpState = LOW;
  // setup medium pump
  pinMode(mediumPumpPin, OUTPUT);
  digitalWrite(mediumPumpPin, LOW);
}

void startAirPump()
{
  // air by pump on/off control
  airPumpState = HIGH;
  // air by pinch valv control
  airValve.write(airValveOpenAngle);
}

void stopAirPump() 
{
  // air by pump on/off control
  airPumpState = LOW;
  // air by pinch valv control
  airValve.write(airValveClosedAngle);
}

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


