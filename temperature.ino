/*
This code was taken from the Arduino forum (arduino.cc).
 It has been modified to meet the project's needs.
 */

// DS18S20 Temperature chip i/o
OneWire owInLiquid(PIN_TEMPERATURE_SENSOR_IN_LIQUID);
DallasTemperature dsInLiquid(&owInLiquid);
DeviceAddress deviceAddressInLiquid;
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
float temperatureInLiquid = 0.0;


void temperatureSetup()
{
//  Serial.println("inited temp.");

  dsInLiquid.begin();
  if (!dsInLiquid.getAddress(deviceAddressInLiquid, 0)) {
    BIOREACTOR_MODE=BIOREACTOR_ERROR_MODE;
//    Serial.println("ERROR in temp. sensor");
  } 
  else {
    dsInLiquid.setWaitForConversion(false);
    dsInLiquid.setResolution(deviceAddressInLiquid, 12);
    dsInLiquid.requestTemperatures();
  }
  // We want to be sure that the conversion has been done
  delay(delayFourhundred);
}

void temperatureUpdate()
{  
  dsInLiquid.requestTemperatures();
  temperatureInLiquid=dsInLiquid.getTempC(deviceAddressInLiquid);
}

float temperatureMeasuredInLiquid()
{
  return temperatureInLiquid;
}
