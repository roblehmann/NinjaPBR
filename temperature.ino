/*-------------------------
management of temperature measurements in 
sample pod water bath
-------------------------*/
// adjust each sensor address according to the individual instances
//Sensor 1
// 28FF34540315024E
//Sensor 2
// 28FF941115150165
//Sensor 3
// 28FF9E32151502C2


OneWire owInLiquid(TEMPERATURE_SENSOR_PIN);
DallasTemperature dsInLiquid(&owInLiquid);
//DeviceAddress tempAdd[3];
byte tempAdd[3][8] = {{ 0x28, 0xFF, 0x34, 0x54, 0x03, 0x15, 0x02, 0x4E },
  { 0x28, 0xFF, 0x94, 0x11, 0x15, 0x15, 0x01, 0x65 }, 
  { 0x28, 0xFF, 0x9E, 0x32, 0x15, 0x15, 0x02, 0xC2 }};
/*-------------------------
setup temperature measurement
-------------------------*/
void temperatureSetup()
{ 
  dsInLiquid.begin();
  
  int tempSensCount = dsInLiquid.getDeviceCount();
  if(tempSensCount < numChambers)
  {
      BIOREACTOR_MODE=BIOREACTOR_ERROR_MODE;
      sendError(F("temp. sensor not found"));
  } else for(int i=0; i<numChambers; i++)
        dsInLiquid.setResolution(tempAdd[i], 12);

  dsInLiquid.requestTemperatures();
}

void temperatureUpdate()
{  
  dsInLiquid.requestTemperatures();
  delay(400);
  for(int i=0; i<numChambers; i++)
    temperatureInLiquid[i] = dsInLiquid.getTempC(tempAdd[i]);
}
