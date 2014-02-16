// timer id for next wavelengths measurement (need to wait until each measurement is finished, and start afterwards)
int nextWavelengthReadTimerID = -1;

Adafruit_TSL2561_Unified tsl1 = NULL;
Adafruit_TSL2561_Unified tsl2 = NULL;
Adafruit_TSL2561_Unified tsl3 = NULL;
// setup for gain 0:1x, 1:16x, 2: autogain
int gain = 0;
// setup for sensor integration period 0:13ms, 1:101ms, 2:402ms
int integrationTime = 2;
//-------------------------------------------------//
//-------------------------------------------------//
// put emitter in default mode (off)
void odSetup()
{
  //  setup emitters
  pinMode(ir850Pin, OUTPUT);
  digitalWrite(ir850Pin, LOW);    
  pinMode(ir740Pin, OUTPUT);
  digitalWrite(ir740Pin, LOW);
  pinMode(redPin, OUTPUT);
  digitalWrite(redPin, LOW);    
  pinMode(greenPin, OUTPUT);
  digitalWrite(greenPin, LOW);    
  pinMode(bluePin, OUTPUT);
  digitalWrite(bluePin, LOW);    
  
  //------------------------------------------
  //  setup detectors
  tsl1 = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT,1);
  setupSensor(tsl1);
  if(numChambers > 1)
  {
    tsl2 = Adafruit_TSL2561_Unified(TSL2561_ADDR_HIGH, 2);
    setupSensor(tsl2);
  }
  if(numChambers > 2) 
  {
    tsl3 = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 3);
    setupSensor(tsl3);
  }
 }

//-------------------------------------------------//
//-------------------------------------------------//
// configure the sensors
void setupSensor(Adafruit_TSL2561_Unified tsl)
{
  /* You can also manually set the gain or enable auto-gain support */
  switch (gain) {
    case 0:
      tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
      break;
    case 1:
      tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
      break;
    default: 
      tsl.enableAutoGain(true);          /* Auto-gain ... switches automatically between 1x and 16x */
  } 
    
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  switch (integrationTime) {
    case 0:
      tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
      break;
    case 1:
      tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
      break;
    default: 
      tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
  }
}
//-------------------------------------------------//
//-------------------------------------------------//
// function is repeatedly called for different wavelelengths;
// switches emitter on/off if necessary and reads signal in all available chambers,
// and calls itself for next wavelength until all are measured
void odUpdate()
{
  // ------------------------------------
  // reading process 
  // read intensities from each chamber for current wavelength
  //---------------
  // A) switch on emitter if necessary (nLambda = measure background, stored in col 5)
  if(nLambda > -1)
    digitalWrite(ledPins[nLambda], HIGH);
  //---------------
  // B) initiate measuring process in all chambers
//  for(int iChamber=0; iChamber < numChambers; iChamber++)
//    measureInChamber(iChamber);
  startTSLsensors();
  //---------------
  // C) wait until each sensor finished measurement
  waitForTSLconversion();
  //---------------
  // D) poll resulting measurements and stop sensors
  fetchTSLsensorData();
  stopTSLsensors();
  //---------------
  // E) switch off emitter again if necessary
  if(nLambda > -1)
    digitalWrite(ledPins[nLambda], LOW);


  //---------------
  // calling next wavelengths reading process
  if(nLambda >= (numLeds-1)) // done reading available wavelengths - finish OD reading
  {
    odUpdateStop();  // put back pump in orig mode and switch light on
  }
  else // read OD and go to next wavelength
  {
    nLambda += 1;
    // finally, call this function after waiting period for the next wavelength
    odUpdate();
  }
}

//-------------------------------------------------//
//-------------------------------------------------//
// function tells TSL sensors in all chambers to start measuring
void startTSLsensors()
{
  if(DEBUG) Serial.println("Starting TSLs");
  tsl1.enable();
  if(numChambers > 1) tsl2.enable();
  if(numChambers > 2) tsl3.enable();
}
//-------------------------------------------------//
//-------------------------------------------------//
// function tells TSL sensors in all chambers to stop measuring
void stopTSLsensors()
{
  if(DEBUG) Serial.println("Stopping TSLs");
  tsl1.disable();
  if(numChambers > 1) tsl2.disable();
  if(numChambers > 2) tsl3.disable();
}

//-------------------------------------------------//
//-------------------------------------------------//
// function causes delay to allow TSL to measure, according to the employed integration time
void waitForTSLconversion()
{
    if(DEBUG) Serial.println("Waiting for TSLs");
    switch (integrationTime) {
    case 0:
      /*13ms: fast but low resolution */
      delay(100);
      break;
    case 1:
      /* 101ms: medium resolution and speed   */
      delay(190);
      break;
    default: 
      /* 402ms: 16-bit data but slowest conversions */
      delay(490);
    }
}
//-------------------------------------------------//
//-------------------------------------------------//
// function fetches measurement results from TSL sensors in all chambers
void fetchTSLsensorData()
{
  if(DEBUG) Serial.println("Fetching TSLs data");
  uint16_t bb, ir; 
  //  chamber 1
  tsl1.fetchData(&bb, &ir);
  storeTSLdata(0, bb, ir);
  //  chamber 2 if present  
  if(numChambers > 1)
  {
    tsl2.fetchData(&bb, &ir);
    storeTSLdata(1, bb, ir);
  }
  //  chamber 3 if present
  if(numChambers > 2)
  {
    tsl3.fetchData(&bb, &ir);
    storeTSLdata(2, bb, ir);
  }
}
//-------------------------------------------------//
//-------------------------------------------------//
// function manages storage of intensity values
void storeTSLdata(int iChamber, uint16_t bb, uint16_t ir)
{
  uint16_t val;
  if(nLambda <=1) val = bb;
  else val = bb - ir;
    // need to check if background / reference value / OD is measured and do according calculation
  if(readReferenceValues)
  {
    if(nLambda == -1) // ref background values, no need to bg subtraction
      refValues[iChamber][5]  = val;
    else // store reference intensity, not OD
      refValues[iChamber][nLambda] = abs(val  - refValues[iChamber][5]); 
  }
  else
  {
    if(nLambda == -1) // background values, no need to bg subtraction
      odValues[iChamber][5]  = val;
    else // normal OD values
      odValues[iChamber][nLambda] = -log(abs(val  - odValues[iChamber][5]) / refValues[iChamber][nLambda]); 
  }
}

