void pumpSetup()
{
  // setup pump
  airPumpState = LOW;
  pinMode(pumpPin, OUTPUT);
  analogWrite(pumpPin, 0);
  Serial.println("inited pump");
}

void startAirPump()
{
  airPumpState = HIGH;
  analogWrite(pumpPin, pumpSpeed);
}

void stopAirPump() 
{
  airPumpState = LOW;
  analogWrite(pumpPin, 0);
}




