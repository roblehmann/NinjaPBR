// timer id for next wavelengths measurement (need to wait until each measurement is finished, and start afterwards)
int nextWavelengthReadTimerID = -1;

//Adafruit_TSL2561_Unified tsl1 = NULL;
//Adafruit_TSL2561_Unified tsl2 = NULL;
//Adafruit_TSL2561_Unified tsl3 = NULL;
TSL2561 tsl1 = NULL; 
TSL2561 tsl2 = NULL; 
TSL2561 tsl3 = NULL; 


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
//  digitalWrite(ir850Pin, LOW);    
  analogWrite(ir850Pin, 0);
  pinMode(ir740Pin, OUTPUT);
//  digitalWrite(ir740Pin, LOW);
  analogWrite(ir740Pin, 0);
  pinMode(redPin, OUTPUT);
//  digitalWrite(redPin, LOW);    
  analogWrite(redPin, 0);
  pinMode(greenPin, OUTPUT);
//  digitalWrite(greenPin, LOW);    
  analogWrite(greenPin, 0);
  pinMode(bluePin, OUTPUT);
//  digitalWrite(bluePin, LOW);    
  analogWrite(bluePin, 0);
  
  //------------------------------------------
  //  setup detectors
//  tsl1 = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW,1);
  tsl1 = TSL2561(TSL2561_ADDR_LOW);
  setupSensor(tsl1);
  if(numChambers > 1)
  {
//    tsl2 = Adafruit_TSL2561_Unified(TSL2561_ADDR_HIGH, 2);
    tsl2 = TSL2561(TSL2561_ADDR_HIGH);
    setupSensor(tsl2);
  }
  if(numChambers > 2) 
  {
//    tsl3 = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT,3);
    tsl3 = TSL2561(TSL2561_ADDR_FLOAT);
    setupSensor(tsl3);
  }
 }

//-------------------------------------------------//
//-------------------------------------------------//
// configure the sensors
void setupSensor(TSL2561 tsl)
{
  /* You can also manually set the gain or enable auto-gain support */
  switch (gain) {
    case 0:
      tsl.setGain(TSL2561_GAIN_0X);      /* No gain ... use in bright light to avoid sensor saturation */
      break;
    default:
      tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  } 
    
  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  switch (integrationTime) {
    case 0:
      tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
      break;
    case 1:
      tsl.setTiming(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
      break;
    default: 
      tsl.setTiming(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */
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
    analogWrite(ledPins[nLambda], emitterBrightness[nLambda]);
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
    analogWrite(ledPins[nLambda], 0);


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
  tsl1.enable();
  if(numChambers > 1) tsl2.enable();
  if(numChambers > 2) tsl3.enable();
}
//-------------------------------------------------//
//-------------------------------------------------//
// function tells TSL sensors in all chambers to stop measuring
void stopTSLsensors()
{
  tsl1.disable();
  if(numChambers > 1) tsl2.disable();
  if(numChambers > 2) tsl3.disable();
}

//-------------------------------------------------//
//-------------------------------------------------//
// function causes delay to allow TSL to measure, according to the employed integration time
void waitForTSLconversion()
{
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
uint16_t bb, ir;
void fetchTSLsensorData()
{
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
  // TODO: test effect of IR subtraction on the resulting value (does IR yield usable correction term for high-pass filter)
    // need to check if background / reference value / OD is measured and do according calculation
  if(readReferenceValues)
  {
    if(nLambda == -1) // ref background values, no need to bg subtraction
      refValues[iChamber][5]  = bb;
    else // store reference intensity, not OD
      refValues[iChamber][nLambda] = abs(bb  - refValues[iChamber][5]); 
  }
  else
  {
    if(nLambda == -1) // background values, no need to bg subtraction
      odValues[iChamber][5]  = bb;
    else // normal OD values
      odValues[iChamber][nLambda] = -log10(abs(bb  - odValues[iChamber][5]) / refValues[iChamber][nLambda]); 
  }
}

