void loggingSetup()
{
  Serial.println("Logging successfully initialized.");
}

const String sep = ",";

void loggingEvent()
{
//  if (DEBUG) Serial.println("logging...");

  /*
  Log all variable data from all sensors 
   print out the following: TimeStamp[sec],ReactorID[ID],TempSet[C],TempInLiquid[C],TempBottomPlate[C],TempAmbient[C],
   HeatingPIDregOutut[0; 5000],LiquidLevel[inch],LiquidIN[secON/60sec],LiquidOUT[secON/60sec],pH[1;14],AcidIN[secON/60sec],
   BaseIn[secON/60sec],MotorState[OFF;ON],NH4[concentr],GazCH4IN[secON/60sec],GazCO2IN[secON/60sec],BioreactorState[0:2]
   */

   // send to computer
  Serial.print("DATA,");
  Serial.print(temperatureMeasuredInLiquid());
  Serial.print(sep);
  Serial.print(od());
  Serial.print(sep);
  Serial.print(odBg());
  Serial.print(sep);
  Serial.print(od2());
  Serial.print(sep);
  Serial.print(od2Bg());
  Serial.print(sep);
  Serial.print(od3());
  Serial.print(sep);
  Serial.print(od3Bg());
  Serial.print(sep);
//  Serial.print(PHASE_NAMES[dayPhase-1]);
  Serial.print(dayPhase);
  Serial.print(sep);
  Serial.print(lightBrightness);
  Serial.print(sep);
//  Serial.print(MODE_NAMES[BIOREACTOR_MODE-1]);
  Serial.print(BIOREACTOR_MODE);
  // end line
  Serial.println();
}








