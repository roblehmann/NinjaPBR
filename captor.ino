// CAPTOR
// Cyanobacterial Arduino-based PhotobioreacTOR
//
// bioreactor firmware for the Arduino-based photobioreactor v0.1
// Author: r.lehmann@biologie.hu-berlin.de

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Timer.h>
#include <Servo.h>

//----------CONSTANTS-GLOBAL--------- //
#define BIOREACTOR_STANDBY_MODE   0   // nothing on, nothing measured
#define BIOREACTOR_LIGHT_MODE     1   // light const. at full brightness, od/temp. measured
#define BIOREACTOR_DARK_MODE      2   // light off, od/temp. measured
#define BIOREACTOR_DYNAMIC_MODE   3   // light varies according to uploaded dynamic profile, od/temp. measured
#define BIOREACTOR_ERROR_MODE     4   // something went wrong,switch off everything

//---------- CONSTANTS-DETEKTOR -----//
#define  odPin      A0  // OD detector diode
#define  odPin2     A1  // OD detectpor diode
//----------CONSTANTS-LIGHT--------- //
#define  ledPin     3  // LED panel connected to digital pin 3
//----------CONSTANTS- EMITTER --------- //
#define  redPin     8  //Red-Emitter diode
#define  greenPin   9  //green-Emitter diode
#define  bluePin    10  //blue-Emitter diode
#define  ir740Pin   11  //IR-Emitter diode
#define  ir850Pin   12  //IR-Emitter diode
//----------CONSTANTS-GAS--------- //
#define  airValvePin 5
//----------CONSTANTS-MEDIUM--------- //
#define  mediumPumpPin 6


const int ledPins[] = {
  ir850Pin, ir740Pin, redPin, greenPin, bluePin};

// marks default state of timer
#define timerNotSet -1
// used often as delay and for conversion
#define thousand 1000L

//----------CONSTANTS-TEMPERATURE--------- //
#define PIN_TEMPERATURE_SENSOR_IN_LIQUID  2   // temperature on culure bottle

//----------TIMER DECLARATION--------- //
Timer t;

//----------GLOBAL VARIABLES--------- //
//global variables used by functions and are set by GUI
byte BIOREACTOR_MODE = BIOREACTOR_STANDBY_MODE;
byte bioreactorAncientMode = BIOREACTOR_STANDBY_MODE;

// LIGHT REGULATION //
byte minLightBrightness = 0;
byte maxLightBrightness = 255;

byte lightBrightness = 0;       // holds brightness of LED panel in range 0-255 (0=off)

// Light profile //
const int      lightProfileLength = 200;
byte           brightnessValue[lightProfileLength];
unsigned long  brightnessDuration[lightProfileLength];
int            lightProfileIdx; // indicates the current position in the light profile 

// Turbidostat Mode //
// if active, captor checks of optical density exceeds "upperOdThr". if so, a
// series of dilution steps of length dilutionDuration are issued, until the OD
// is found below "lowerOdThr"
boolean  inTurbidostat    = false;   // is turbidostat mode is active
boolean  inDilution       = false;   // is turbidostat mode is active
int      dilutionDuration = 1;       // how long the medium pump should run to dilute one step (sec)
float    upperOdThr       = 1;       // when to start diluting
float    lowerOdThr       = .8;      // when to stop diluting

// OD MEASUREMENTS //
// foreground od measurements
#define numLeds 5
#define numReadingsAverage 8.0
#define valDiv 1023.0
// ir850, ir740, red, green, blue
float od1Values[5];
float od2Values[5];

//reference values for OD calculation
// ir850, ir740, red, green, blue
float od1RefValues[5] = {
  5,5,5,5,5};
float od2RefValues[5] = {
  5,5,5,5,5};

// background od measurements
float odValBg = 0.0;
float odVal2Bg = 0.0;

// state flag, determining whether to measure reference value for OD calculation or OD values
boolean readReferenceValues = false;

// AIR PUMP REGULATION //
boolean airPumpState;
Servo airValve;
#define airValveOpenAngle 8
#define airValveClosedAngle 64

// how often to measure od/temp
unsigned long sensorSamplingTime = 10L; //seconds

// store timer IDs, to be able to remove them when the BIOREACTOR_MODE or the sample timer change
int lightChangeTimerID = -1;
int sensorReadTimerID = -1;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
// for logging and communication
const char* sep = ",";

//----------------------------- //
//----------setting up--------- //
//----------------------------- //
void setup()  {
  // for serial input message
  inputString.reserve(50);
  // init serial connection to controlling computer
  Serial.begin(9600);
  lightSetup();
  temperatureSetup();
  odSetup();
  pumpSetup();
  // update all the sensor data for once before doing the first log
  updateSensorLogValues();

  bioreactorAncientMode = BIOREACTOR_STANDBY_MODE;
  BIOREACTOR_MODE = BIOREACTOR_STANDBY_MODE;
} 

//----------------------------- //
//----------loop--------- //
//----------------------------- //
void loop()  { 
  // update the timer
  t.update();
  // read new configuration from Serial
  checkSerialMessage();
  // check if reactor mode changed and make changes effective
  checkChangedReactorMode();
}


void checkChangedReactorMode () 
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
        // deactivate air pump 
        stopAirPump();
        // switch off light, remove timers
        stopSensorReadTimer();
        lightOff();
        break;
      }
    case BIOREACTOR_DYNAMIC_MODE:
      {
        // activate air pump 
        startAirPump() ;
        // ------------ Adding timed actions ----------
        startSensorReadTimer();
        startDynamicLightTimers();
        break;
      }
    case BIOREACTOR_LIGHT_MODE:
      {
        startAirPump() ;
        lightOn();
        startSensorReadTimer();
        break;
      }
    case BIOREACTOR_DARK_MODE:  
      {
        startAirPump() ;
        lightOff();
        startSensorReadTimer();
        break;
      }
    case BIOREACTOR_ERROR_MODE:
      {
        stopAirPump() ;
        lightOff();
        stopSensorReadTimer();
        break;
      }
    default:
      BIOREACTOR_MODE = BIOREACTOR_ERROR_MODE;
      stopAirPump() ;
      lightOff();
      stopSensorReadTimer();
      break;
    }
    sendMode();
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
    if (inChar == '#') stringComplete = true;
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

char   charBuf[30];
String paramName;
float  paramValue;

/* method takes message from controlling computer and parses it, assigning the 
 command value to the corresponding variable. 
 messages structure:
 parameterName:parameterValue#
 */
void digestMessage() 
{
  inputString.toCharArray(charBuf, 30); 
  paramName = strtok(charBuf,sep);
  paramValue = atof(strtok(NULL,sep));
  Serial.println(paramName);
  Serial.println(paramValue);

  if(paramName == "md") // which reactor program to use
  {
    BIOREACTOR_MODE = paramValue;
  }
  else if(paramName == "sst") // how often to sample
  {
    sensorSamplingTime = paramValue;
    if(sensorReadTimerID != timerNotSet) // if sampling is currently active, make new frequency active
    {
      stopSensorReadTimer(); // restart sampling timer
      startSensorReadTimer(); // standby mode and reinit reactor mode
    }
  }
  else if(paramName == "malb") // maximal brightness of the LED panel
  {
    maxLightBrightness = paramValue;
    lightUpdate();
  }
  else if(paramName == "milb") // maximal brightness of the LED panel
  {
    minLightBrightness = paramValue;
    lightUpdate();
  }
  else if(paramName == "mre") // measure reference values for OD calculation
  {
    // set flag to read reference values (without OD calculation)
    readReferenceValues = true;
    odUpdate();
    readReferenceValues = false;
    // send reference values to interface
    sendReferenceValues();
  }
  else if(paramName == "bp") // receive brightness profile values
  {
    if(paramValue >= 0 & paramValue < lightProfileLength)
    {
      // first value is the index, second is the brightness (0-255), third is the duration in seconds
      brightnessValue[int(paramValue)] = atoi(strtok(NULL,sep));
      brightnessDuration[int(paramValue)] = atol(strtok(NULL,sep));
    } 
    else {
      Serial.println("invalid idx");
    }
  }  
  else if(paramName == "it") // set turbidostat mode. if paramValue==1, activate, otherwise deactivate
  {
    inTurbidostat = (paramValue==1);
  }
  else if(paramName == "dd") // set length of dilution (how long the medium pump runs in seconds)
  {
    dilutionDuration = paramValue;
  }
  else if(paramName == "uot") // OD when to start dilution
  {
    upperOdThr = paramValue;
  }
  else if(paramName == "lot") // OD when to stop dilution
  {
    lowerOdThr = paramValue;
  }
  else if(paramName == "sbp") // receive brightness profile values
  {
    sendLightProfile();
  }
  else
  {
    Serial.print("unknown");
    Serial.println(paramName);
  }
}










