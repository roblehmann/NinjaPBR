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
  Serial.println("The temperature has been initialized.");

  dsInLiquid.begin();
  if (!dsInLiquid.getAddress(deviceAddressInLiquid, 0)) {
    BIOREACTOR_MODE=BIOREACTOR_ERROR_MODE;
    Serial.println("ALERT: Can not determine temperature of liquid");
  } 
  else {
    dsInLiquid.setWaitForConversion(false);
    dsInLiquid.setResolution(deviceAddressInLiquid, 12);
    dsInLiquid.requestTemperatures();
  }

  // We want to be sure that the conversion has been done
  delay(400);
}

void temperatureUpdate()
{  
  dsInLiquid.requestTemperatures();
  temperatureInLiquid=dsInLiquid.getTempC(deviceAddressInLiquid);
//  if(DEBUG)
//  {
//    Serial.print("The measured temperature of the liquid is: "); 
//    Serial.println(temperatureInLiquid);
//  }  
}

float temperatureMeasuredInLiquid()
{
  return temperatureInLiquid;
}
