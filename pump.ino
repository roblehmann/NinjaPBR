void pumpSetup()
{
  // setup pump
  airPumpState = LOW;
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, airPumpState);
  Serial.println("inited pump");
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

