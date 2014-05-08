void loggingSetup()
{
  // nothing to do for me here...
}

void loggingEvent()
{
  // send message type
  Serial.print("DATA;");
  
  // send intensity values
  for(int j=0; j<numChambers; j++)
  {
    for(int i=0; i < numLeds; i++)
    {
      Serial.print(od1Values[j][i]);
      Serial.print(sep);
    }
    for(int i=0; i < numLeds; i++)
    {
      Serial.print(od2Values[j][i]);
      if(!((j == numChambers-1) && (i == numLeds-1))) 
        Serial.print(sep);
    }
  }
  Serial.print(';');
  
  // send background intensity values 
  for(int j=0; j<numChambers; j++)
  {
    Serial.print(odValBg[j]);
    Serial.print(sep);
    Serial.print(odVal2Bg[j]);
    if(j<numChambers-1) Serial.print(sep);
  }
  Serial.print(';');
  
  //  send other parameters
  Serial.print(temperatureMeasuredInLiquid());
  Serial.print(sep);
  Serial.print(lightBrightness);
  Serial.print(sep);
  Serial.print(BIOREACTOR_MODE);
  Serial.print(sep);
  Serial.print(minLightBrightness);
  Serial.print(sep);
  Serial.print(maxLightBrightness);
  Serial.print(sep);
  Serial.print(sensorSamplingTime);  
  Serial.println();
}

// announce reference values used for OD calculation
void sendReferenceValues()
{
  Serial.print("REF;");
  for(int j=0; j<numChambers; j++)
  {
    for(int i=0; i < numLeds; i++)
    {
      Serial.print(od1RefValues[j][i]);
      Serial.print(sep);
    }
    for(int i=0; i < numLeds; i++)
    {
      Serial.print(od2RefValues[j][i]);
      Serial.print(sep);
    }
  }
  Serial.println();
}

// announce dynamic light profile values
void sendLightProfile()
{
  Serial.print("LP;");
  for(int i=0;i<lightProfileLength;i++)
  {
    Serial.print(brightnessValue[i]);
    Serial.print(sep);
    Serial.print(brightnessDuration[i]);
    Serial.print(sep);
  }
  Serial.println();
}


// announce current CAPTOR mode
void sendMode()
{
  Serial.print("MD;");
  Serial.print(BIOREACTOR_MODE);
  Serial.println(sep);
}






