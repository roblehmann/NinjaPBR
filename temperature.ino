/*
This code was taken from the Arduino forum (arduino.cc).
 It has been modified to meet the project's needs.
 */

// DS18S20 Temperature chip i/o
OneWire owInLiquid(PIN_TEMPERATURE_SENSOR_IN_LIQUID);
DallasTemperature dsInLiquid(&owInLiquid);
DeviceAddress tempAdd[3];

/*-------------------------
setup temperature measurement
-------------------------*/
void temperatureSetup()
{
  dsInLiquid.begin();
  for(int i=0; i<numChambers; i++)
    if (!dsInLiquid.getAddress(tempAdd[i], i)) {
      BIOREACTOR_MODE=BIOREACTOR_ERROR_MODE;
      sendError(F("temp. sensor not found"));
    } 
    else 
    {
  //    dsInLiquid.setWaitForConversion(false);
      dsInLiquid.setResolution(tempAdd[i], 12);
      dsInLiquid.requestTemperatures();
    }
  // We want to be sure that the conversion has been done
  delay(400);
}

void temperatureUpdate()
{  
  dsInLiquid.requestTemperatures();
  for(int i=0; i<numChambers; i++)
    temperatureInLiquid[i] = dsInLiquid.getTempC(tempAdd[i]);
}
