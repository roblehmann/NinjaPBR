/*-------------------------
// NinjaPBR
// Arduino-based Photobioreactor for the cultivation of cyanobacteria and microalgae
//
// Author: r.lehmann@biologie.hu-berlin.de
-------------------------*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Timer.h>
#include <Wire.h>
#include <TSL2561_mod.h>
#include <DS3232RTC.h>
#include <Time.h>
#include <UTFT.h>
#include <UTouch.h>
#include <UTFT_Buttons.h>
#include <SPI.h>
#include <SdFat.h>

//----------CONSTANTS-GLOBAL--------- //
#define BIOREACTOR_STANDBY_MODE   0   // nothing on, nothing measured
#define BIOREACTOR_LIGHT_MODE     1   // light const. at full brightness, od/temp. measured
#define BIOREACTOR_DARK_MODE      2   // light off, od/temp. measured
#define BIOREACTOR_DYNAMIC_MODE   3   // light varies according to uploaded dynamic profile, od/temp. measured
#define BIOREACTOR_ERROR_MODE     4   // something went wrong,switch off everything

//----------CONSTANTS-LIGHT--------- //
//#define  ledPin     13  // LED panel connected to digital pin 3
const int lightPins[] = {11,12,13};

#define MAX_LIGHT     0   // set illumination to specified max
#define MIN_LIGHT     1   // set illumination to specified min

//----------CONSTANTS- EMITTER --------- //
#define numLeds 1     // number of emitters with different wavelength
// number of culture chambers with individual OD sensors
// needs to match the number of OD sensor pins per type
const int ledPins[] = {8, 10};

//----------Number of Pods --------- //
#define numChambers 3

//----------CONSTANTS-GAS--------- //
#define  airValvePin 9
//----------CONSTANTS-MEDIUM--------- //
#define  mediumPumpPin 6

// marks default state of timer
#define timerNotSet -1

//----------CONSTANTS-TEMPERATURE--------- //
#define TEMPERATURE_SENSOR_PIN  19   // culture temperature

//----------TIMER DECLARATION--------- //
Timer t;

//----------GLOBAL VARIABLES--------- //
//global variables used by functions and are set by GUI
byte BIOREACTOR_MODE = BIOREACTOR_STANDBY_MODE;
byte bioreactorAncientMode = BIOREACTOR_STANDBY_MODE;

// LIGHT REGULATION //
byte minLightBrightness[3] = {0};
byte maxLightBrightness[3] = {255, 255, 255};

byte lightBrightness[3] = {0};       // holds brightness of LED panel in range 0-255 (0=off)

// Light profile //
const int      lightProfileLength = 200;
byte           brightnessValue[3][lightProfileLength];
unsigned long  brightnessDuration[3][lightProfileLength];
int            lightProfileIdx[3] = {0}; // indicates the current position in the light profile 

// Turbidostat Mode //
// if active, reactor checks of optical density exceeds "upperOdThr". if so, a
// series of dilution steps of length dilutionDuration are issued, until the OD
// is found below "lowerOdThr"
boolean  inTurbidostat    = false;   // is turbidostat mode is active
boolean  inDilution       = false;   // is turbidostat mode is active
int      dilutionDuration = 1;       // how long the medium pump should run to dilute one step (sec)
float    upperOdThr       = 1;       // when to start diluting
float    lowerOdThr       = .8;      // when to stop diluting

// OD MEASUREMENTS //
// array storing the resulting OD values for logging
// column 5 stores background values, column 0-4 the background-corrected OD values
float odValues[numChambers][ 6 ] = {0};

//reference values for OD calculation
// column 5 stores background values, column 0-4 the background-corrected reference intensity values
float refValues[numChambers][ 6 ] = {65535, 65535, 65535, 65535, 65535, 65535};

// emitter-specific brightness value. 
// adjust this value to optimize for individual geometry
// set to 100 for integration time 2 and sfh4550
float emitterBrightness[ 5 ] = {100,162,255,255,255};

// state flag, determining whether to measure reference value for OD calculation or OD values
boolean readReferenceValues = false;
//flag indicating if reference values were read already
String referenceValuesDone = "";

// AIR PUMP REGULATION //
boolean airPumpState;

// how often to measure od/temp
unsigned long sensorSamplingTime = 5L; //seconds

// store timer IDs, to be able to remove them when the BIOREACTOR_MODE or the sample timer change
int lightChangeTimerID[] = {-1, -1, -1};
int sensorReadTimerID = -1;

float temperatureInLiquid[3] = {0, 0, 0};

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
// for logging and communication
const char* sep = ",";
String sepT = "/";  //separator char for logging and display
String curr_t; // holds time stamp of current sample

// SD logging vars
#define FILE_BASE_NAME "NPLog"
char fileName[13] = FILE_BASE_NAME "00.csv";
bool SDavail = false;
bool SDlogging = false;
//send handshakes until connected
bool serialConnected = false;

const int line_buffer_size = 140;
char buffer[line_buffer_size];
String paramName;
float  paramValue;


//----------------------------- //
//----------setting up--------- //
//----------------------------- //
void setup()  {
  // for serial input message
  inputString.reserve(50);

  // initialize reactor mode and functions
  bioreactorAncientMode = BIOREACTOR_STANDBY_MODE;
  BIOREACTOR_MODE = BIOREACTOR_STANDBY_MODE;
  
  initSerial();
  lightSetup();
  temperatureSetup();
  odSetup();
  pumpSetup();
  loggingSetup();
  setupDisplay();
  // update all the sensor data for once before doing the first log
  updateSensorLogValues();
} 

//----------------------------- //
//----------loop--------- //
//----------------------------- //
void loop()  { 
  // update the timer
  t.update();
  //check if mode change seleceted via display
  check_settings_selected();
  // read new configuration from Serial
  checkSerialMessage();
  // check if reactor mode changed and make changes effective
  checkChangedReactorMode();
}


void checkChangedReactorMode() 
{
  // check if reactor mode changed, and put changes into effect
  if(bioreactorAncientMode != BIOREACTOR_MODE)  
  {
    bioreactorAncientMode = BIOREACTOR_MODE;
    switch(BIOREACTOR_MODE)
    {
    case BIOREACTOR_STANDBY_MODE: 
      { 
        // to go into standby, need to switch off light, remove timers for day phase/light change
        // open pinch valves - permant close-state causes damage
        startAirPump();
        // switch off light, remove timers
        stopSensorReadTimer();
        setMaxMinLight(MIN_LIGHT);
        break;
      }
    case BIOREACTOR_DYNAMIC_MODE:
      {
        // activate air pump 
        startAirPump();
        // ------------ Adding timed actions ----------
        startSensorReadTimer();
        startDynamicLightTimers();
        break;
      }
    case BIOREACTOR_LIGHT_MODE:
      {
        startAirPump();
        setMaxMinLight(MAX_LIGHT);
        startSensorReadTimer();
        Serial.println(F("Set Light Mode"));
        break;
      }
    case BIOREACTOR_DARK_MODE:  
      {
        startAirPump();
        setMaxMinLight(MIN_LIGHT);
        startSensorReadTimer();
        break;
      }
    case BIOREACTOR_ERROR_MODE:
      {
        stopAirPump();
        setMaxMinLight(MIN_LIGHT);
        stopSensorReadTimer();
        break;
      }
    default:
      BIOREACTOR_MODE = BIOREACTOR_ERROR_MODE;
      startAirPump();
      setMaxMinLight(MIN_LIGHT);
      stopSensorReadTimer();
      break;
    }
    sendMode();
    showData();
  }
}

/*
method is called when message comes via usb. incoming messages
 are collected until message terminating character is see, then 
 digestion is triggered
 */
void serialEvent() {
  while (Serial.available()) {
    // read new byte
    char inChar = (char) Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // character "#" marks end of message, if seen trigger digestion
    if (inChar == '#') 
      stringComplete = true;
    else // wait for serial buffer, otherwise get overflows and thus dropped messages from computer 
      delay(20);
  }
}


void checkSerialMessage() {
  if (stringComplete) {
    if (inputString != "")       // change setup according to message
      digestMessage();
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}

/*-------------------------------- 
 method takes message from controlling computer and parses it, assigning the 
 command value to the corresponding variable. 
 messages structure:
 parameterName:parameterValue#
 --------------------------------*/
void digestMessage() 
{
  inputString.toCharArray(buffer, line_buffer_size); 
  paramName = strtok(buffer,sep);
  paramValue = atof(strtok(NULL,sep));

  if(paramName == F("md")) // which reactor program to use
  {
    BIOREACTOR_MODE = paramValue;
    prepareBoxDisplay();
  }
  else if(paramName == F("sst")) // how often to sample
  {
    sensorSamplingTime = paramValue;
    if(sensorReadTimerID != timerNotSet) // if sampling is currently active, make new frequency active
    {
      stopSensorReadTimer(); // restart sampling timer
      startSensorReadTimer(); // standby mode and reinit reactor mode
    }
  }
  else if(paramName == F("malb")) // maximal brightness of the LED panel
  {
    maxLightBrightness[0] = paramValue;
    for(int i=1; i < numChambers; i++)
    {
      maxLightBrightness[i] =  atoi(strtok(NULL,sep));
    }
    for(int i=0; i < numChambers; i++)
      lightUpdate(i);
  }
  else if(paramName == F("milb")) // minimal brightness of the LED panel
  {
    
    minLightBrightness[0] = paramValue;
    for(int i=1; i < numChambers; i++)
    {
      minLightBrightness[i] = atoi(strtok(NULL,sep));
    }
    for(int i=0; i < numChambers; i++)
      lightUpdate(i);
  }
  else if(paramName == F("mre")) // measure reference values for OD calculation
  {
    // set flag to read reference values (without OD calculation)
    readReferenceValues = true;
    odUpdateStart();
  }
  else if(paramName == F("bp")) // receive brightness profile values
  {
    if( paramValue >= 0 && paramValue < numChambers) // check if valid chamber is addressed
    {
      // first value is the channel, second the index, third is the brightness (0-255), fourth is the duration in seconds
      int idx = atoi(strtok(NULL,sep));
      if(idx >= 0 && idx < lightProfileLength)
      {
        brightnessValue[int(paramValue)][idx]     = atoi(strtok(NULL,sep));
        brightnessDuration[int(paramValue)][idx]  = atol(strtok(NULL,sep));
        Serial.print('MSG,');
        Serial.print((String) brightnessValue[int(paramValue)][idx] );
        Serial.print(' ');
        Serial.print((String)brightnessDuration[int(paramValue)][idx]);
        Serial.print(' ');
        Serial.println(idx);
      }
      else 
      {
        sendError(F("invalid light profile idx"));
      }
    } 
    else {
      sendError(F("invalid channel"));
    }
  }  
  else if(paramName == F("it")) // set turbidostat mode. if paramValue==1, activate, otherwise deactivate
  {
    inTurbidostat = (paramValue==1);
  }
  else if(paramName == F("dd")) // set length of dilution (how long the medium pump runs in seconds)
  {
    dilutionDuration = paramValue;
  }
  else if(paramName == F("uot")) // OD when to start dilution
  {
    upperOdThr = paramValue;
  }
  else if(paramName == F("lot")) // OD when to stop dilution
  {
    lowerOdThr = paramValue;
  }
  else if(paramName == F("sbp")) // receive brightness profile values
  {
    sendLightProfile(paramValue);
  }
  else if(paramName == F("hi")) // acknowledge serial connect
  {
    //set back flag to stop sending handshakes again
    serialConnected = true;
    StopSendingHandshake();
  }
  else if(paramName == F("bye")) // acknowledge serial disconnect
  {
    //set back flag to start sending handshakes again
    serialConnected = false;
    StartSendingHandshake();
  }
  else if(paramName == F("log")) // request to send all log data from current file
  {
    sendLogData();
  }
  else
  {
    sendError("unknown message: " + paramName);
  }
  saveParameters();
}











