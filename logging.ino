/*-------------------------
management of data logging in controller (to SD) 
and transfer to computer (via USB)
-------------------------*/

// rounding accuracy
int nPositions = 6;
// for SD card initialization
const uint8_t chipSelect = SS;
// file system object
SdFat sd;
// log file object
SdFile file;
char paramStoreFile[11]  = "params.txt";
char paramStoreFile2[11] = "lghtps.txt";

// temporary variables for parameter reading from storage file
float resArr[numChambers + 2][6] = {0};  
float line[6] = {0};
char  cs[6];

/*----------------------- 
logging setup
-----------------------*/
void loggingSetup()
{
  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if(timeStatus() != timeSet) 
    sendError(F("Unable to sync with the RTC"));
//  else
//    Serial.println(F("RTC has set the system time"));
  SDavail = initSD();
  loadParameters();
}

/*----------------------- 
logging via serial connection
-----------------------*/
void loggingEvent()
{
  logToSerial();
  logToSD();
}

/*----------------------- 
send current data to computer via serial
-----------------------*/
void logToSerial()
{
    // send message type
  Serial.print("DATA;");
  
  // send intensity values
  for(int j=0; j<numChambers; j++)
  {
    for(int i=0; i < numLeds; i++)
    {
      Serial.print(odValues[j][i],nPositions);
      if(!((j == numChambers-1) && (i == numLeds-1))) 
        Serial.print(sep);
    }
  }
  Serial.print(';');
  
  // send background intensity values 
  for(int i=0; i<numChambers; i++)
  {
    Serial.print(odValues[i][5],nPositions);
    if(i<numChambers-1) Serial.print(sep);
  }
  Serial.print(';');
  //  send temperature values
    for(int i=0; i<numChambers; i++)
  {
    Serial.print(temperatureInLiquid[i]);
    if(i<numChambers-1) Serial.print(sep);
  }
  Serial.print(';');
  // send light values
  for(int i=0; i<numChambers; i++)
  {
    Serial.print(lightBrightness[i]);
    if(i<numChambers-1) Serial.print(sep);
  }
  Serial.print(';');
  //send min light values
  for(int i=0; i<numChambers; i++)
  {
    Serial.print(minLightBrightness[i]);
    if(i<numChambers-1) Serial.print(sep);
  }
  Serial.print(';');
  //send max light values
  for(int i=0; i<numChambers; i++)
  {
    Serial.print(maxLightBrightness[i]);
    if(i<numChambers-1) Serial.print(sep);
  }
  Serial.print(';');
  Serial.print(sensorSamplingTime);  
  Serial.print(sep);
  Serial.print(curr_t);
  Serial.print(sep);
  Serial.println(BIOREACTOR_MODE);
}

/*----------------------- 
write current data to file on SD, if available
-----------------------*/
void logToSD()
{
  // check if SD logging is available
  if(!SDavail | !SDlogging)
    return;
  
  //log measured values 
  if (!file.open(fileName, O_CREAT | O_APPEND | O_WRITE))
  {
    sendError(F("error SD file.open"));
    return;
  } 

    // send message type
  file.print("DATA;");
  
  // send intensity values
  for(int j=0; j<numChambers; j++)
  {
    for(int i=0; i < numLeds; i++)
    {
      file.print(odValues[j][i],nPositions);
      if(!((j == numChambers-1) && (i == numLeds-1))) 
        file.print(sep);
    }
  }
  file.print(';');
  
  // send background intensity values 
  for(int i=0; i<numChambers; i++)
  {
    file.print(odValues[i][5],nPositions);
    if(i<numChambers-1) file.print(sep);
  }
  file.print(';');
  //  send temperature values
    for(int i=0; i<numChambers; i++)
  {
    file.print(temperatureInLiquid[i]);
    if(i<numChambers-1) file.print(sep);
  }
  file.print(';');
  // send light values
  for(int i=0; i<numChambers; i++)
  {
    file.print(lightBrightness[i]);
    if(i<numChambers-1) file.print(sep);
  }
  file.print(';');
  //send min light values
  for(int i=0; i<numChambers; i++)
  {
    file.print(minLightBrightness[i]);
    if(i<numChambers-1) file.print(sep);
  }
  file.print(';');
  //send max light values
  for(int i=0; i<numChambers; i++)
  {
    file.print(maxLightBrightness[i]);
    if(i<numChambers-1) file.print(sep);
  }
  file.print(';');
  file.print(sensorSamplingTime);  
  file.print(sep);
  file.print(curr_t);
  file.print(sep);
  file.println(BIOREACTOR_MODE);
  
  if (!file.sync() || file.getWriteError()) {
    sendError(F("SD write error"));
  }
  file.close();
}
/*----------------------- 
announce reference values used for OD calculation
-----------------------*/
void sendReferenceValues()
{
  Serial.print("REF;");
  for(int j=0; j<numChambers; j++)
  {
    for(int i=0; i < numLeds; i++)
    {
      Serial.print(refValues[j][i]);
      Serial.print(sep);
    }
  }
  Serial.println();
}

/*----------------------- 
announce dynamic light profile values
-----------------------*/
void sendLightProfile(int chamber)
{
  Serial.print(F("LP;"));
    for(int i=0;i<lightProfileLength;i++)
    {
      Serial.print(brightnessValue[chamber][i]);
      Serial.print(sep);
      Serial.print(brightnessDuration[chamber][i]);
      Serial.print(sep);
    }
  Serial.println();
}

/*----------------------- 
announce current NinjaPBR mode
-----------------------*/
void sendMode()
{
  Serial.print(F("MD;"));
  Serial.print(BIOREACTOR_MODE);
  Serial.println(sep);
}

/*-----------------------
Initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
 breadboards. Use SPI_FULL_SPEED for better performance.
-----------------------*/
boolean initSD()
{
  boolean res = sd.begin(chipSelect, SPI_HALF_SPEED);
  if(!res)
    sendMessage("could not init SD");
  return(res);
}


/*----------------------- 
create new log file name with consecutive numbering
-----------------------*/
boolean findLogName()
{
  boolean res = true;
  // init SD card for logging
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    sendError("FILE_BASE_NAME too long");
  }
  
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      sendError("Can't create file name");
      res = false;
    }
  }
  return(res);
}

/*----------------------- 
test writing to the specified log file
-----------------------*/
boolean testLogFile()
{
  if (!file.open(fileName, O_CREAT | O_WRITE)) {
    sendError(F("error SD file.open"));
    return(false);
  } 
  else
  {
    file.close();
    String s = "using SD log" + String(fileName);
    sendMessage(s);
    return(true);
  }
}

/*----------------------- 
tell computer about something
-----------------------*/
void sendMessage(String msg)
{
  Serial.print(F("MSG;"));
  Serial.println(msg); 
}

/*----------------------- 
tell computer about error
-----------------------*/
void sendError(String msg)
{
  Serial.print(F("ERROR;"));
  Serial.println(msg); 
}

/*----------------------- 
send the current data log to the computer
-----------------------*/  
void sendLogData()
{
  if(!SDavail)
  {
    sendMessage(F("SD card not initialized, no LOG available"));
    return;
  }
  sendMessage(fileName);
  ifstream sdin(fileName);

  while (sdin.getline(buffer, line_buffer_size, '\n') || sdin.gcount()) 
  {
    if (sdin.fail()) {
      // Partial long line
      sdin.clear(sdin.rdstate() & ~ios_base::failbit);
    } else if (sdin.eof()) {
      // Partial final line  // sdin.fail() is false
    }else
      Serial.println((String)buffer);
  }
}

/*-------------------
load last used box parameters in case of restart
------------------- */
boolean loadParameters() 
{
    if(!SDavail)
  {
    sendMessage(F("SD card not initialized"));
    return(false);
  }
  
  /*------------------------
  load reference values and light parameters
  ------------------------*/
  // open input file
  ifstream sdin(paramStoreFile);

  // check for open error
  if (!sdin.is_open()) {
    sendError(F("Can not open parameter file"));
  }

  // read until input fails
  for(int i=0; i<numChambers + 2; i++) 
  {
    // Assume EOF if fail.
    if (sdin.fail())
      break;
    // Get commas and numbers.
    sdin >> line[0] >> cs[0] >> line[1]>> cs[1] >> line[2] >> cs[2] >> line[3]>> cs[3] >> line[4] >> cs[4] >> line[5] >> cs[5];
    // Skip CR/LF.
    sdin.skipWhite();
    if (sdin.fail())
      sendError(F("Bad input in parameter file"));
    // error in line if not commas
//      if (cs[j] != sep)
//        sendError(F("Comma"));
    // print in six character wide columns
    for(int j=0; j<6; j++)
      resArr[i][j] = line[j];
  }

  // brightness values for constant light modes
  for(int i=0; i<3; i++)
  {
    minLightBrightness[i] = resArr[0][i];
    maxLightBrightness[i] = resArr[0][i+3];
  }
  
  // set last reference values
  for(int i=0; i<numChambers; i++)
    for(int j=0; j<6; j++)
    {
      refValues[i][j] = resArr[numChambers][j];
    }

  // sampling frequency
  sensorSamplingTime = resArr[numChambers + 1][0];

  // current position in the light cycle of the dynamic light program
  for(int j=0; j<3; j++)
    lightProfileIdx[j] = resArr[numChambers + 1][j+1];
    
  // check last state of SD logging. if was active and SD is available, set to active again
  if(SDavail)
  {
    SDlogging = (boolean) resArr[numChambers + 1][4];
  }
  
  /*-------------------------
  load dynamic light programs
  --------------------------*/  
    // open input file
  ifstream sdin2(paramStoreFile2);

  // check for open error
  if (!sdin2.is_open()) {
    sendError(F("Can not open dynamic light program file"));
  }

  // read until input fails
  for(int i=0; i<lightProfileLength; i++)
  {
    // Assume EOF if fail.
    if (sdin2.fail())
      break;
    // Get commas and numbers.
    sdin2 >> line[0] >> cs[0] >> line[1]>> cs[1] >> line[2] >> cs[2] >> line[3]>> cs[3] >> line[4] >> cs[4] >> line[5] >> cs[5];
    // Skip CR/LF.
    sdin2.skipWhite();
    if (sdin2.fail())
      sendError(F("Bad input in dynamic light file"));
    
    for(int j=0; j<3; j++) 
    {
      brightnessValue[j][i]     = line[j];
      brightnessDuration[j][i]  = line[j+3];
    }
  }
  
  return(true);
}

/*-------------------
persists current box parameters in case of restart
------------------- */
boolean saveParameters() 
{

    if(!SDavail)
  {
    sendError(F("SD card not initialized"));
    return(false);
  }
    /*-------------------
  saving parameters and reference values
  --------------------*/
  if (!file.open(paramStoreFile, O_CREAT | O_WRITE)) 
  {
    sendError(F("error writing params to file"));
    return(false);
  }

  for(int i=0; i<3; i++) 
  {
    file.print(minLightBrightness[i]);
    file.print(sep);
  }
  for(int i=0; i<3; i++) 
  {
    file.print(maxLightBrightness[i]);
    file.print(sep);
  }
  file.println();
  for(int i=0; i<numChambers; i++) 
  {
    for(int j=0; j<6; j++) 
    {
      file.print(refValues[i][j]);
      file.print(sep);
    }
    file.println();
  }
  file.print(sensorSamplingTime);
  file.print(sep);
  for(int j=0; j<3; j++)
  {
      file.print(lightProfileIdx[j]);
      file.print(sep);
  }
  // save SD logging state
  file.print((int)SDlogging);
  file.println(F(",-1,"));
  file.print(F("NinjaPBR Parameters, "));
  file.println(curr_t);
  // Force data to SD and update the directory entry to avoid data loss.
  if (!file.sync() || file.getWriteError()) 
  {
    sendError("SD write error");
  }
  file.close();

  /*-------------------
  saving light programs
  --------------------*/
  if (!file.open(paramStoreFile2, O_CREAT | O_WRITE)) 
  {
    sendError(F("error writing light programs to file"));
    return(false);
  }

  for(int i=0; i<lightProfileLength; i++) 
  {
    for(int j=0; j<3; j++) 
    {
      file.print(brightnessValue[j][i]);
      file.print(sep);
    }
    for(int j=0; j<3; j++) 
    {
      file.print(brightnessDuration[j][i]);
      file.print(sep);
    }
    file.println();
  }
  file.close();
  
  return(true);
}
