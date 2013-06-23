void pumpSetup()
{
  airValve.attach(airValvePin);
  // setup pump
  airPumpState = LOW;
  pinMode(pumpPin, OUTPUT);
  analogWrite(pumpPin, 0);
  Serial.println("inited pump");
}

void startAirPump()
{
  // air by pump on/off control
  airPumpState = HIGH;
  analogWrite(pumpPin, pumpSpeed);
  // air by pinch valv control
  airValve.write(airValveOpen);
}

void stopAirPump() 
{
  // air by pump on/off control
  airPumpState = LOW;
  analogWrite(pumpPin, 0);
  // air by pinch valv control
  airValve.write(airValveClosed);
}




