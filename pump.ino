void pumpSetup()
{
  // setup pump
  airPumpState = LOW;
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, airPumpState);
  Serial.println("The air pump has been initialized.");
}

void startAirPump() 
{
  airPumpState = HIGH;
  digitalWrite(pumpPin, airPumpState);

}

void stopAirPump() 
{
  airPumpState = LOW;
  digitalWrite(pumpPin, airPumpState);

}

