extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t Dingbats1_XL[];


UTFT          myGLCD(ITDB32S,38,39,40,41);
UTouch        myTouch(6,5,4,3,2);
UTFT_Buttons  myButtons(&myGLCD, &myTouch);

int displayWidth, displayHeight;

// Diese Buttons werden in allen Menuebenen verwendet
unsigned int main_but1, main_but2, main_but3, main_but4, main_but5;
/* m_press der aktuell geklickte Button im Hauptmenu
kann auch -1 für kein Button sein, daher int */
int m_press;

int background_col_light[3]    = {200, 200, 0};
int background_col_dark[3]     = {0, 0, 255  };
int background_col_dyn[3]      = {0, 161, 58 };
int background_col_standby[3]  = {0, 0, 100  };
int background_col_error[3]    = {200, 0, 0  };

/*-------------------
 do all preparation to use the display
------------------- */
void setupDisplay()
{
  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);
//  myGLCD.setFont(BigFont);
  myTouch.InitTouch(LANDSCAPE);
  myTouch.setPrecision(PREC_MEDIUM);
  
  myButtons.setTextFont(BigFont);
  myButtons.setSymbolFont(Dingbats1_XL);
  displayWidth = myGLCD.getDisplayXSize();
  displayHeight = myGLCD.getDisplayYSize();
  
  curr_t = "";
  curr_t = curr_t + hour() + sepT + minute() + sepT + second() + sepT + day() + sepT + month() + sepT + year();
  prepareBoxDisplay();
}

/* ----------------------------
put all text on the display that is replaced with every sample
----------------------------*/
void showData()
{
  int startX = 90;
  int startY = 50;
  for(int j=0; j<numChambers; j++)
  {
    // show OD values
    for(int i=0; i < numLeds; i++)
    {
      myGLCD.print((String)odValues[j][i], startX + 50*j, startY + 15*i);
    }
    // show reference values
    myGLCD.print((String)(long)refValues[j][0], startX + 50*j, 110);
    // show background values
    myGLCD.print((String)(long)odValues[j][5], startX + 50*j, 125);
  }
 
  for(int i=0; i<numChambers; i++)
  {
    myGLCD.print("   ", startX + 45*i, 140);
    myGLCD.print((String) temperatureInLiquid[i], startX + 45*i, 140);
  }

  startX = 120;
  myGLCD.print((String)BIOREACTOR_MODE, startX, 155);
  
  for(int i=0; i<numChambers; i++)
  {
    myGLCD.print("   ", startX + 30*i, 170);
    myGLCD.print((String)lightBrightness[i], startX + 30*i, 170);
  }
    
  if(SDlogging)
    myGLCD.print(fileName, startX, 185);
  else
    myGLCD.print(F("Off"), startX, 185);
  
  // system time
  myGLCD.print(F("                 "), 100, 225);
  myGLCD.print(curr_t, 100, 225);
}

/* ----------------------------
put all text on the display that doesnt have to be replaced with every sample
----------------------------*/
void prepareBoxDisplay()
{

  myButtons.deleteAllButtons();
  myGLCD.clrScr();
  setBoxModeBgCol();
  myGLCD.setFont(BigFont);
  myGLCD.print("Captor Photobioreactor", 5, 10);
  myGLCD.setFont(SmallFont);
  
  myGLCD.print(F("OD[a.u.]:"), 10, 50);
  myGLCD.print(F("REF[a.u.]:"), 10, 110);
  myGLCD.print(F("BG[a.u.]:"), 10, 125);
  myGLCD.print(F("T[C]:"), 10, 140);
  myGLCD.print(F("Reactor Mode:"), 10, 155);
  myGLCD.print(F("Light Val.:"), 10, 170);
  myGLCD.print(F("SD Logging:"), 10, 185);
  showData();
  add_settings_button();
}

/*--------------------------- 
adjust screen background color according to activity mode
---------------------------*/

void setBoxModeBgCol()
{
  if(BIOREACTOR_MODE == BIOREACTOR_LIGHT_MODE)
    myGLCD.fillScr(background_col_light[0], background_col_light[1], background_col_light[2]);
  else if(BIOREACTOR_MODE == BIOREACTOR_DARK_MODE)
    myGLCD.fillScr(background_col_dark[0], background_col_dark[1], background_col_dark[2]);
  else if(BIOREACTOR_MODE == BIOREACTOR_DYNAMIC_MODE)
    myGLCD.fillScr(background_col_dyn[0], background_col_dyn[1], background_col_dyn[2]);
  else if(BIOREACTOR_MODE == BIOREACTOR_STANDBY_MODE)
    myGLCD.fillScr(background_col_standby[0], background_col_standby[1], background_col_standby[2]);
  else if(BIOREACTOR_MODE == BIOREACTOR_ERROR_MODE)
    myGLCD.fillScr(background_col_error[0], background_col_error[1], background_col_error[2]);
}

/*-----------------------------------
show operation mode menu screen
-----------------------------------*/
void mode_set_screen()
{
  myButtons.deleteAllButtons();
  myGLCD.clrScr();
  setBoxModeBgCol();
  // Buttons des Hauptmenu setzen
  main_but1 = myButtons.addButton(10,10,300,40,"Standby");
  main_but2 = myButtons.addButton(10,50,300,40,"Light");
  main_but3 = myButtons.addButton(10,90,300,40,"Dark");
  main_but4 = myButtons.addButton(10,130,300,40,"Dyn.Light");
  main_but5 = myButtons.addButton(10,170,300,40,"Back");
  // Buttons anzeigen
  myButtons.drawButtons();
  // Einen Text schreiben
  myGLCD.setColor(VGA_WHITE);
  while(true)
  {
    // Hier beginnt die Abfrage der geklickten Buttons
    if (myTouch.dataAvailable())
    {
      m_press = myButtons.checkButtons();
      if (m_press==main_but1) 
        BIOREACTOR_MODE = BIOREACTOR_STANDBY_MODE;
      else if(m_press==main_but2)
        BIOREACTOR_MODE = BIOREACTOR_LIGHT_MODE;
      else if(m_press==main_but3) 
        BIOREACTOR_MODE = BIOREACTOR_DARK_MODE;
      else if(m_press==main_but4) 
        BIOREACTOR_MODE = BIOREACTOR_DYNAMIC_MODE;
      else if(m_press==main_but5) //return to base screen
        ;
      // always return to previous screen after button was hit
      return;
    }
  }
}


/*-----------------------------------
show buttons for settings screen
-----------------------------------*/
void drawSettingsScreen()
{
  myButtons.deleteAllButtons();
  myGLCD.clrScr();
  setBoxModeBgCol();
  // Buttons des Hauptmenu setzen
  main_but1 = myButtons.addButton(10,10,300,40,"Min. Light");
  main_but2 = myButtons.addButton(10,50,300,40,"Max. Light");
  main_but3 = myButtons.addButton(10,90,300,40,"Sampling Dist.");
  main_but4 = myButtons.addButton(10,130,300,40,"Logging");
  main_but5 = myButtons.addButton(10,170,300,40,"Back");
  // Buttons anzeigen
  myButtons.drawButtons();
  // Einen Text schreiben
  myGLCD.setColor(VGA_WHITE);
}
/*-----------------------------------
show settings menu screen
-----------------------------------*/
void settings_screen()
{
  drawSettingsScreen();
  while(true)
  {
    // Hier beginnt die Abfrage der geklickten Buttons
    if (myTouch.dataAvailable())
    {
      m_press = myButtons.checkButtons();
      if (m_press==main_but1) 
      {// min light
        int channel  = numberPadButtons(F("Select Reactor Chamber"), numChambers-1, true);
        int entry    = numberPadButtons(F("Enter Min. Light"), 9, false);
        if(entry > -1 & entry < 256)
        {
          minLightBrightness[channel] = entry;
          lightUpdate(channel);
        }
        drawSettingsScreen();
      }
      else if(m_press==main_but2)
      {// max light
        int channel  = numberPadButtons(F("Select Reactor Chamber"), numChambers-1, true);
        int entry    = numberPadButtons(F("Enter Max. Light"), 9, false);
        if(entry > -1 & entry < 256)
        {
          maxLightBrightness[channel] = entry;
          lightUpdate(channel);
        }
        drawSettingsScreen();
      }
      else if(m_press==main_but3) 
      {//samp dist [seconds]
        int entry = numberPadButtons(F("Enter Sampling Distance"), 9, false);
        if(entry > -1)
        {
          sensorSamplingTime = entry;
          if(sensorReadTimerID != timerNotSet) // if sampling is currently active, make new frequency active
          {
            stopSensorReadTimer(); // restart sampling timer
            startSensorReadTimer(); // standby mode and reinit reactor mode
          }
        }
        drawSettingsScreen();
      }
      else if(m_press==main_but4) 
      { // logging to internal SD card
        logging_set_screen();
        drawSettingsScreen();
      }
      else if(m_press==main_but5) //return to base screen
      {
        return;
      }
      saveParameters();
    }
  }
}


/*-----------------------------------
buttons fuer optionen
-----------------------------------*/
void add_settings_button()
{
  myButtons.deleteAllButtons();
  // Buttons des Hauptmenu setzen
  myButtons.setTextFont(SmallFont);
  main_but1 = myButtons.addButton(215,40,100,60,"Mode");
  main_but2 = myButtons.addButton(215,100,100,60,"Settings");
  main_but3 = myButtons.addButton(215,160,100,60,"Reference");
  myButtons.drawButtons();
  myButtons.setTextFont(BigFont);
}

/*-----------------------------------
checks if mode menu has been selected
-----------------------------------*/
void check_settings_selected()
{
  if (myTouch.dataAvailable())
  {
    m_press = myButtons.checkButtons();
    if (m_press==main_but1) 
    {
      mode_set_screen();
      prepareBoxDisplay();
      return;
    } else if (m_press == main_but2) 
    {
      settings_screen();
      prepareBoxDisplay();
      return;
    } else if (m_press == main_but3) 
    {
      // set flag to read reference values (without OD calculation)
      readReferenceValues = true;
      odUpdate();
      saveParameters(); //save new reference values to SD //show new ref val on display
      prepareBoxDisplay();
      return;
    }
  }
}

char stCurrent[20]="";
int stCurrentLen=0;

/*-----------------------------------
number pad for parameter entry
function shows a number pad (0-9) on the display. if less than 9 digits wanted, specify 
largest required number as largestNum. if single number selection is wanted set singleNum flag
-----------------------------------*/
// heading    - explanation of required entry shwn above number pad
// largestNum - largest number possible to select in the number pad, other buttons are disabled
// singleNum  - is true, only one number can be selected from the pad, function immediately returns
int numberPadButtons(String heading, int largestNum, bool singleNum)
{
  myButtons.deleteAllButtons();
  myGLCD.clrScr();
  drawButtonPad(heading, largestNum);
  myButtons.drawButtons();
  int entry = -1;
  stCurrent[0]='\0';
  stCurrentLen=0;
  
  while(true)
  {
    // clicked button check
    if (myTouch.dataAvailable())
    {
      m_press = myButtons.checkButtons();
      switch (m_press) {
        case 0:
          if(singleNum)
            return(0);
          updateStr('0');
          break;
        case 1:
          if(singleNum)
            return(1);
          updateStr('1');
          break;
        case 2:
          if(singleNum)
            return(2);
          updateStr('2');
          break;
        case 3:
          if(singleNum)
            return(3);
          updateStr('3');
          break;
        case 4:
          if(singleNum)
            return(4);
          updateStr('4');
          break;
        case 5:
          if(singleNum)
            return(5);
          updateStr('5');
          break;
        case 6:
          if(singleNum)
            return(6);
          updateStr('6');
          break;
        case 7:
          if(singleNum)
            return(7);
          updateStr('7');
          break;
        case 8:
          if(singleNum)
            return(8);
          updateStr('8');
          break;
        case 9:
          if(singleNum)
            return(9);
          updateStr('9');
          break;
        case 10: // clear button
          stCurrent[0]='\0';
          stCurrentLen=0;
          myGLCD.print(F("                    "), LEFT, 224);        
          break;
        case 11: //enter
          entry = atoi(stCurrent);
          return(entry);
          break;
        case 12: //back
          return(-1);
          break;
        default:
          sendMessage("unkown Button#");
      }
    }
  }
}

/*-----------------------------------
draw number pad
-----------------------------------*/
// heading - shows string above number pad
// largestNum - show numbers until specified number
void drawButtonPad(String heading, int largestNum)
{
  myGLCD.print(heading, 20, 5);
  int firstRowY   = 25;
  int secondRowY  = firstRowY + 65;
  int thirdRowY   = secondRowY + 65;
  myButtons.addButton(250,secondRowY,60,60,"0");
  myButtons.addButton(10,firstRowY,60,60,"1");
  myButtons.addButton(70,firstRowY,60,60,"2");
  myButtons.addButton(130,firstRowY,60,60,"3");
  myButtons.addButton(190,firstRowY,60,60,"4");
  myButtons.addButton(250,firstRowY,60,60,"5");
  myButtons.addButton(10,secondRowY,60,60,"6");
  myButtons.addButton(70,secondRowY,60,60,"7");
  myButtons.addButton(130,secondRowY,60,60,"8");
  myButtons.addButton(190,secondRowY,60,60,"9");
  
  // disable unwanted buttons (need to show them to ensure the button numbering, necessary for evaluation logic)
  for(int i=0; i<=9; i++)
    if(largestNum < i)
      myButtons.disableButton(i, true);
    
  myButtons.addButton(10,thirdRowY,90,60,"Clear");
  myButtons.addButton(110,thirdRowY,90,60,"Enter");
  myButtons.addButton(210,thirdRowY,90,60,"Back");
}
/*-----------------------------------
show pushed button
-----------------------------------*/
void updateStr(int val)
{
  if (stCurrentLen<20)
  {
    stCurrent[stCurrentLen]=val;
    stCurrent[stCurrentLen+1]='\0';
    stCurrentLen++;
    myGLCD.setColor(0, 255, 0);
    myGLCD.print(stCurrent, LEFT, 224);
  }
  else
  {
    myGLCD.setColor(255, 0, 0);
    myGLCD.print("BUFFER FULL!", CENTER, 192);
    delay(500);
    myGLCD.print("            ", CENTER, 192);
    delay(500);
    myGLCD.print("BUFFER FULL!", CENTER, 192);
    delay(500);
    myGLCD.print("            ", CENTER, 192);
    myGLCD.setColor(0, 255, 0);
  }
}

/*-----------------------------------
show operation mode menu screen
-----------------------------------*/
void logging_set_screen()
{
  myButtons.deleteAllButtons();
  myGLCD.clrScr();
  setBoxModeBgCol();
  // Buttons des Hauptmenu setzen
  main_but1 = myButtons.addButton(10,50,300,40,"On");
  main_but2 = myButtons.addButton(10,90,300,40,"Off");
  main_but3 = myButtons.addButton(10,130,300,40,"New File");
  main_but4 = myButtons.addButton(10,170,300,40,"Back ");
  // Buttons anzeigen
  myButtons.drawButtons();
  // Einen Text schreiben
  myGLCD.setColor(VGA_WHITE);
  while(true)
  {
    // wait for buttons to be pressed
    if (myTouch.dataAvailable())
    {
      m_press = myButtons.checkButtons();
      if (m_press == main_but1) // activate logging
      {
        if(SDavail)
          SDlogging = testLogFile();
      }
      else if(m_press == main_but2) // deactivate logging
      {
        SDlogging = false;
      }
      else if(m_press==main_but3)  // new log file
      {
        if(SDavail) // if SD card is installed, find new log file name and test opening it
        {
          findLogName();
          SDlogging = testLogFile();
        }
      }
      else if(m_press==main_but4) // do nothing and return to previous menu
        ;
      // always return to previous screen after button was hit
      return;
    }
  }
}
