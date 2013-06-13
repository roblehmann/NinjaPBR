// foreground od measurements
float odVal = 0.0;
float odVal2 = 0.0;
float odVal3 = 0.0;
// for mean od calc.
float odtmp[3]; 
// background od measurements
float odValBg = 0.0;
float odVal2Bg = 0.0;
float odVal3Bg = 0.0;

void odSetup()
{
  pinMode(irPin, OUTPUT);
  digitalWrite(irPin, LOW);  
  Serial.println("The optical density has been initialized.");
}

// reads OD value and writes in global state variable
void odUpdate()
{ 

  analogWrite(ledPin, 0); 
  digitalWrite(pumpPin, LOW);
  delay(50);
  odValBg =  analogRead(odPin) / 1024.0 * 5.0;
  odVal2Bg =  analogRead(odPin2) / 1024.0 * 5.0;
  odVal3Bg =  analogRead(odPin3) / 1024.0 * 5.0;
  digitalWrite(irPin, HIGH);
  delay(300);
  for(int i=0; i<3; i++) {
    odtmp[i] =  analogRead(odPin) / 1024.0 * 5.0;
  }
  odVal = (odtmp[0] + odtmp[1] + odtmp[2]) / 3;
  odVal2 =  analogRead(odPin2) / 1024.0 * 5.0;
  odVal3 =  analogRead(odPin3) / 1024.0 * 5.0;
  digitalWrite(irPin, LOW);
  digitalWrite(pumpPin, airPumpState); // set pump back to pervious state (can be on/off)
  analogWrite(ledPin, lightBrightness);
  //  if(DEBUG)
  //  {
  //    Serial.print("The measured optical density of the liquid is: "); 
  //    Serial.println(odVal);
  //  }
}

float od()
{
  return odVal;
}

float od2()
{
  return odVal2;
}

float od3()
{
  return odVal3;
}

float odBg()
{
  return odValBg;
}

float od2Bg()
{
  return odVal2Bg;
}

float od3Bg()
{
  return odVal3Bg;
}

