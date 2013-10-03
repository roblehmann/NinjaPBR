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
#define BIOREACTOR_STANDBY_MODE   1   // nothing on, nothing measured
#define BIOREACTOR_LIGHT_MODE     2   // light const. at full brightness, od/temp. measured
#define BIOREACTOR_DARK_MODE      3   // light off, od/temp. measured
#define BIOREACTOR_CIRCADIAN_MODE 4   // light varies circadian, od/temp. measured
#define BIOREACTOR_ERROR_MODE     5  // somewthing went wrong,switch off everything

//----------CONSTANTS-LIGHT--------- //
#define  ledPin     3  // LED panel connected to digital pin 3
//----------CONSTANTS-GAS--------- //
#define  airValvePin 6
//----------CONSTANTS-MEDIUM--------- //
#define  mediumPumpPin 5
//----------CONSTANTS-OD--------- //
#define  ir850Pin      12  //IR-Emitter diode
#define  ir740Pin      8  //IR-Emitter diode
#define  greenPin   7  //Red-Emitter diode
#define  bluePin    9  //Red-Emitter diode
#define  redPin     10  //Red-Emitter diode
#define  odPin      A0  // OD detector diode
#define  odPin2     A1  // OD detectpor diode

const int ledPins[] = {12, 8, 10, 7, 9};

// space saver defines
#define delayFourhundred 400
#define timerNotSet -1
#define thousand 1000L

//----------CONSTANTS-TEMPERATURE--------- //
#define PIN_TEMPERATURE_SENSOR_IN_LIQUID  2   // temperature on culure bottle

//----------CONSTANTS-LIGHT REGULATION ------- //
// status variable marking circadian phase
const byte PHASE_MORNING = 0; // defined as DL transition
const byte PHASE_DAY     = 1; // defined as max. light intensity
const byte PHASE_EVENING = 2; // defined as LD transition
const byte PHASE_NIGHT   = 3; // defined as min. light intensity
const byte PHASE_NONE    = 4; // if no circadian program is used

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
byte lightChangeStep = 25;       // how strong the light intensity changes between two consecutive steps
unsigned long lightChangeStepLength = 1L; // how long should a step to the next light level be  /seconds
byte dayPhase = PHASE_NONE;     // holds whether it is MORNING, EVENING, or NIGHT

// Duration: MORNING (DL transition), DAY (max light), EVENING (LD transition), NIGHT (min light) in SEC!
unsigned long PHASE_DURATIONS[4] = {
  10L, 1L, 10L, 11L};

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
float od1RefValues[5] = {5,5,5,5,5};
float od2RefValues[5] = {5,5,5,5,5};

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

// store timer IDs, to be able to remove them when the BIOREACTOR_MODE or the light-change-params change
int dayPhaseChangeTimerID = -1;
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
    case BIOREACTOR_CIRCADIAN_MODE:
      {
        // activate air pump 
        startAirPump() ;
        // ------------ Adding timed actions ----------
        startSensorReadTimer();
        startCircadianLightTimers();
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
      Serial.println("unknown->ERROR");
      BIOREACTOR_MODE = BIOREACTOR_ERROR_MODE;
      stopAirPump() ;
      lightOff();
      stopSensorReadTimer();
      break;
    }
    loggingEvent();
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

char charBuf[30];
String paramName;
unsigned long paramValue;

/* method takes message from controlling computer and parses it, assigning the 
command value to the corresponding variable. 
messages structure:
parameterName:parameterValue#
*/
void digestMessage() 
{
  inputString.toCharArray(charBuf, 30); 
  paramName = strtok(charBuf,sep);
  paramValue = atol(strtok(NULL,sep));
  Serial.println(paramName);
  Serial.println(paramValue);

  if(paramName == "md") // which reactor program to use
  {
    switch(paramValue)
    {
    case 1:
      BIOREACTOR_MODE = BIOREACTOR_STANDBY_MODE;
      break;
    case 2:
      BIOREACTOR_MODE = BIOREACTOR_LIGHT_MODE;
      break;
    case 3:
      BIOREACTOR_MODE = BIOREACTOR_DARK_MODE;
      break;
    case 4:
      BIOREACTOR_MODE = BIOREACTOR_CIRCADIAN_MODE;
      break;
    case 5:
      BIOREACTOR_MODE = BIOREACTOR_ERROR_MODE;
      break;
    }
  }
  else if(paramName == "lcs") // light change step magnitude
    // brightness increments for morning/evening in circadian mode
    // should be in range 1-255
  {
    if(paramValue < 1) {
      lightChangeStep = 1;
    } 
    else if(paramValue > 255) {
      lightChangeStep = 255;
    } 
    else {
      lightChangeStep = paramValue;
    }
  }
  else if(paramName == "lcsl") // how long to keep each brightness increment
  {
    lightChangeStepLength = paramValue;
  }
  else if(paramName == "ml") // duration of morning, evening, and night
  {
    PHASE_DURATIONS[PHASE_MORNING] = paramValue;
  }
  else if(paramName == "dl") // duration of morning, evening, and night
  {
    PHASE_DURATIONS[PHASE_DAY] = paramValue;
  }
  else if(paramName == "el") // duration of morning, evening, and night
  {
    PHASE_DURATIONS[PHASE_EVENING] = paramValue;
  }
  else if(paramName == "nl") // duration of morning, evening, and night
  {
    PHASE_DURATIONS[PHASE_NIGHT] = paramValue;
  }
  else if(paramName == "sst") // how often to sample
  {
    sensorSamplingTime = paramValue;
    stopSensorReadTimer(); // restart sampling timer, to avoid having to go to 
    startSensorReadTimer(); // standby mode and reinit reactor mode
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
  else if(paramName == "ddl") // do dilution for turbidostat mode
  {
    // provided parameter is length in seconds for the medium pump to run
    doDilution(paramValue);
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
  else
  {
    Serial.print("unknown");
    Serial.println(paramName);
  }
}







