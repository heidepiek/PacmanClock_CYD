/*
  Licensed as Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  You are free to:
  Share — copy and redistribute the material in any medium or format
  Adapt — remix, transform, and build upon the material
  Under the following terms:
  Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.
  NonCommercial — You may not use the material for commercial purposes.
  ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original
*/

#include "PMclockMenu.h"

int ssidSelectIndex = 0;
bool ssidSelected = false;
char tempSSID[32];
char tempPW[32];
bool changedSSID = false;
bool changedPW = false;
bool changedWiFiEnable = false;

#define INPUT_STR_SZ 16
int inputStrPtr = 0;
char inputStr[INPUT_STR_SZ + 2];
String kbTitle;

#define REGION_COUNT 10

bool regionSelected = false;
int regionSelectIndex = 0;
int startRegion = 0;
int startCity = 0;

struct RECORD_STRUCT {
  int rSize;
  char rData[66];
} record;


//
//  Time Zone Configuration data, saved in SPIFFS, restored on boot/reboot
//
struct TZ_CONFIG_STRUCT {
  int regionIndx;
  int cityIndx;
  char tzInfo[48];
} tzConfig, tzTemp;


const char* regionName[REGION_COUNT] = { "Africa", "Americas", "Antarctica", "Asia",
                                         "Atlantic", "Australia", "Etc", "Europe", "Indian", "Pacific",
                                       };
struct CITY_STRUCT {
  char cityName[48];
  char tzInfo[48];
} city, changedCity;

int cityCount = 0;
int cityPage = 0;
int citySelectIndex = 0;
bool citySelected = false;
int changedCityIndx = 0;
int changedRegionIndx = 0;
char changedTzInfo[48];

File fTXT;

struct FILE_INDEX_STRUCT {
  int recordStart;
  int recordSize;
} recordIndxTbl[160];


bool getRecord(int recordIndx);
int indexFile(void);
//int getRecordCount(char *fileName);

int fileSize = 0;
int recordNbr = 0;

bool configChange = false;
int pwKeypad = LC_KEYPAD;
bool keypadBtn3UC = false;
int menuState = MAIN_MENU;


/////////////////////////////////////////////////////////////////////
//
//  Setup Clock Menu (main menu)
//
//  Menu to setup Clock Wifi, Time Parameters, Alarm and Pacman/ Ms Pacman option
//
void setupclockmenu()
{
  PACMAN_PRINTLN(F("Setup Menu"));
  //UpdateDisp();         // update value to clock
  bool done = false;
  configChange = false;
  pushTA();             // Push Text Attributes on attribute stack
  displayMainMenu();    // sets menuState = MAIN_MENU
  while (!done)
  {
    audioPlayer();      // check to see if anything is playing
    if (touchCheck())       
    {
      Serial.printf("menuState %d\n\r", menuState);
      switch (menuState)
      {
        case MAIN_MENU:
          done = processMainMenu();
          break;
        case ALARM_MENU:
          if (processAlarmMenu())
          {
            displayMainMenu();
          }
          else displayAlarmMenu();
          break;
        case WIFI_MENU:
          if (processWiFiMenu() == true)    // Done or Exit
          {
            displayMainMenu();
          }
          //else displayWiFiMenu();
          break;
        case WIFI_CONFIG_MENU:
          if (processWiFiConfigMenu() == true)    // Done or Exit
          {
            displayWiFiMenu();
          }
          break;
        case SSID_MENU:
          if (processSsidMenu())    // Exit or Done ?
          {
            //if (ssidSelected)       // if changed/selected SSID
            if (changedSSID)       // if changed/selected SSID
            {
              //strncpy(WiFiConfig.ssID, tempSSID, 32);
              //ssidSelected = false;
            }
            displayWiFiConfigMenu();
          }
          break;
        case PW_MENU:
          if (processPwMenu())    // Done ?
          {
            //if (passSelected)     // if Password was entered
            if (changedPW)      // if Password was entered
            {
              strncpy(tempPW, inputStr, 32);
              //strncpy(WiFiConfig.passWord, tempPW, 32);
              //passSelected = false;
              //strncpy(tempPW, inputStr, 32);
              //tempPW = String(inputStr);
            }
            displayWiFiConfigMenu();
          }
          break;
        case TZ_MENU:
          if (processTzMenu())    // Exit ?
          {
            //if (TZSelected)     // if Timezone was entered
            //{
            //TZSelected = false;
            //}
            displayMainMenu();
          }
          break;
        case REGION_MENU:
          if (processRegionMenu())    // Exit ?
          {
            displayTzMenu();
          }
          break;
        case CITY_MENU:
          if (processCityMenu())    // Exit ?
          {
            displayTzMenu();
          }
          break;
        case ABOUT_MENU:
        {
          if(processAboutMenu())
          {
            displayMainMenu();
          }
          //displayAboutMenu();
          break;
        }
        default:
          break;
      }
      touchData.touchedFlag = false;
      delay(250);
    }
  }   //while(!done)
//
//  done == true
//
  if (configChange) wrtFile(fnClockConfig, (char *)&clockConfig, sizeof(clockConfig));
  //* Clear Screen
  tft.fillRect(0, 0, 320, 240, TFT_BLACK);
  xsetup = true; // Set Flag now leaving setup mode in order to draw Clock Digits
  popTA();          // Pop Text Attributes from attribute stack
  drawscreen(); // Initiate the screen
  UpdateDisp(); // update value to clock
}

///////////////////////////////////////////////////
//
//  Display Main Menu
//
void displayMainMenu(void)
{
  menuState = MAIN_MENU;
  // Push Text Attributes on attribute stack
  pushTA();
  // Clear screen and setup layout of main clock setup screen - Wifi, Time Zone, Alarm and Character menus
  drawFullScreenOutline();
//  tft.setTextColor(TFT_WHITE, TFT_BLACK);
//  tft.setTextSize(1);          // Text size multiplier
//  tft.fillScreen(TFT_BLACK);
//  // ---------------- Outside wall
//  tft.drawRoundRect(0  , 0 , 319 , 239 , 2 , TFT_YELLOW);
//  tft.drawRoundRect(2  , 2 , 315 , 235 , 2 , TFT_YELLOW);
  // Setup Main Menu buttons
  drawBtn(1, TFT_WHITE, "Time", "Zone");
  drawBtn(4, TFT_WHITE, "WiFi", "");
  drawBtn(3, TFT_WHITE, "Alarm", "");
  displayClock12_24();
  drawBtn(12, TFT_WHITE, "Exit", "");
  drawBtn(7, TFT_WHITE, "Play", "Tune");
  // Setup Pac-man speed button
  if (clockConfig.speedSetting == 1) { // flag where false is off and true is on
    drawBtn(9, TFT_WHITE, "Normal", "");
  } else if (clockConfig.speedSetting == 2) { // flag where false is off and true is on
    drawBtn(9, TFT_WHITE, "Fast", "");
  } else if (clockConfig.speedSetting == 3) { // flag where false is off and true is on
    drawBtn(9, TFT_WHITE, "Crazy!!", "");
  }
  drawBtn(11, TFT_WHITE, "About", "");
  // Display Ghosts (just decoration)
  tft.pushImage(120, 50, 28, 28, redGhostEyesRight);
  tft.pushImage(170, 50, 28, 28, blueGhost);
  // Display MS Pacman or Pacman in Button #6
  drawBtn(6, TFT_WHITE, "", "");
  tft.pushImage(250, 75, 28, 28, pacmanOpenRight);
  // Pop Text Attributes from attribute stack
  popTA();
}

////////////////////////////////////////////////////
//
//  Process Main Menu (screen touches)
//
bool processMainMenu(void)
{
  bool done = false;
  switch (touchData.button)
  {
    case BUTTON12:     // Exit
      playGobble = true;  // Play button confirmation sound
      done = true;
      break;
    case BUTTON11:    // About
      displayAboutMenu();
      break;
    case BUTTON3:   // Alarm Menu
      playGobble = true;  // Play button confirmation sound
      displayAlarmMenu();
      break;
    case BUTTON6:   // Mr. or Ms. Pac-man
      playGobble = true;  // Play button confirmation sound
      if (clockConfig.mspacman)
      {
        clockConfig.mspacman = false;
        loadPacmanIcons();
      }
      else
      {
        clockConfig.mspacman = true;
        loadMsPacmanIcons();
      }
      tft.pushImage(250, 75, 28, 28, pacmanOpenRight);
      configChange = true;
      break;
    case BUTTON7:     // Play Tune
      playPM = true;  // Play Pac-man Theme
      break;
    case BUTTON9:     // Pac-man speed
      clockConfig.speedSetting++;
      // Speed setting must be in range 1 to 3
      if (clockConfig.speedSetting == 4) clockConfig.speedSetting = 1;
      // Display the speed in button
      if (clockConfig.speedSetting == 1) { // flag where false is off and true is on
        drawBtn(9, TFT_WHITE, "Normal", "");
      } else if (clockConfig.speedSetting == 2) { // flag where false is off and true is on
        drawBtn(9, TFT_WHITE, "Fast", "");
      } else if (clockConfig.speedSetting == 3) { // flag where false is off and true is on
        drawBtn(9, TFT_WHITE, "Crazy!!", "");
      }
      setGameSpeed();  
      playGobble = true;  // Play button confirmation sound
      configChange = true;
      break;
    case BUTTON1:           // Time Zone Menu
      // copy configured time zone info to temp tz info
      tzTemp.regionIndx = tzConfig.regionIndx;
      tzTemp.cityIndx = tzConfig.cityIndx;
      strncpy(tzTemp.tzInfo, tzConfig.tzInfo, 48);
      displayTzMenu();
      break;
    case BUTTON4:           // WiFi Menu
      strncpy(tempSSID, WiFiConfig.ssID, 32);
      strncpy(tempPW, WiFiConfig.passWord, 32);
      changedSSID = false;
      changedPW = false;
      changedWiFiEnable = false;
      displayWiFiMenu();
      inputStrPtr = 0;
      inputStr[0] = 0;
      break;
    case BUTTON10:        // 12 / 24 hour time display
      if (clockConfig.clock24) clockConfig.clock24 = false;
      else clockConfig.clock24 = true;
      displayClock12_24();
      configChange = true;
      break;
    default:
      break;
  }
  return done;
}

////////////////////////////////////////////////
//
//  Display About Menu
//
void displayAboutMenu(void)
{
  menuState = ABOUT_MENU;
  // Menu used to view Clock f/w version
  PACMAN_PRINTLN(F("About Menu"));
  // Push Text Attributes on attribute stack
  pushTA();
  // Clear screen and setup layout of About menu
  drawFullScreenOutline();
  setGfxFont(FSS9);
  tft.setTextFont(2);
  tft.setTextSize(1);      // Text size multiplier
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString("About", 160, pgLine1); //GFXFF);
  tft.setTextDatum(TL_DATUM);        // Left Justify Text (default)
  tft.drawString("File:  " FILE_NAME, pgStart, pgLine2);  //GFXFF);
  tft.drawString("Compiled:  " __DATE__ ", " __TIME__ ", " __VERSION__, pgStart, pgLine3);
  tft.drawString(ideVer, pgStart, pgLine4);
  tft.drawString("IP Address:   " + ipAddr, pgStart, pgLine5);
  tft.drawString("MAC Address:  " + macAddr, pgStart, pgLine6);

  drawBtn(12, TFT_WHITE, "Exit", ""); 
  popTA();
}

bool processAboutMenu(void)
{
  bool results = false;         // returns true if exiting Alarm Menu
  switch (touchData.button)
  {
    case BUTTON12:     // About Menu EXIT
      //playGobbleSound(); // Play button confirmation sound
      //playGobble = true;
      //PACMAN_PRINTLN(F("Exit"));
      results = true;
      break;
    default:
      break;
  }
  return results;    
}


////////////////////////////////////////////////
//
//  Display Alarm Menu
//
void displayAlarmMenu(void)
{
  menuState = ALARM_MENU;
  // Menu used to set the Alarm time
  PACMAN_PRINTLN(F("Alarm Menu"));
  // Push Text Attributes on attribute stack
  pushTA();
  // Clear screen and setup layout of Alarm menu
  drawFullScreenOutline();
  //Display Current Alarm Setting
  displayAlarmSetting();
  // Pop Text Attributes from attribute stack
  popTA();
}

/////////////////////////////////////////////////////
//
//  Process Alarm Menu (screen touches)
//
bool processAlarmMenu(void)
{
  bool results = false;         // returns true if exiting Alarm Menu
  // Capture input command from user
  switch (touchData.button)
  {
    case BUTTON1:     // Alarm ON
      playGobble = true;  // Play button confirmation sound
      clockConfig.alarmStatus = ALARM_ENABLED;    //!clockConfig.alarmStatus;
      configChange = true;    // returns true if exiting Alarm Menu
      break;
    case BUTTON2:     // Increment Alarm HOUR
      clockConfig.alarmHour++;
      if (clockConfig.alarmHour > 24)
      {
        clockConfig.alarmHour = 1;
      }
      configChange = true;
      break;
    case BUTTON5:   // Decrement Alarm HOUR
      clockConfig.alarmHour--;
      if (clockConfig.alarmHour < 1)
      {
        clockConfig.alarmHour = 24;
      }
      configChange = true;
      break;
    case BUTTON3:   // Increment Alarm MINUTE
      clockConfig.alarmMinute++;
      if (clockConfig.alarmMinute > 59) {
        clockConfig.alarmMinute = 0;
      }
      configChange = true;
      break;
    case BUTTON4:     // Alarm OFF
      playGobble = true;  // Play button confirmation sound
      clockConfig.alarmStatus = ALARM_DISABLED;   //!clockConfig.alarmStatus;
      configChange = true;    // returns true if exiting Alarm Menu
      break;
    case BUTTON6:     // Decrement Alarm MINUTE
      clockConfig.alarmMinute--;
      if (clockConfig.alarmMinute < 0)
      {
        clockConfig.alarmMinute = 59;
      }
      configChange = true;
      break;
    case BUTTON7:     // Alarm Volume UP
      if (clockConfig.alarmVolume < 100) clockConfig.alarmVolume += 10;
      DacAudio.DacVolume = clockConfig.alarmVolume;
      playGobble = true;  // Play button confirmation sound
      Serial.print("Volume ");\
      Serial.println(clockConfig.alarmVolume);
      configChange = true;
      break;
      break;
    case BUTTON10:    // Alarm Volume DOWN
      if (clockConfig.alarmVolume > 0) clockConfig.alarmVolume -= 10;
      DacAudio.DacVolume = clockConfig.alarmVolume;
      playGobble = true;  // Play button confirmation sound
      Serial.print("Volume ");\
      Serial.println(clockConfig.alarmVolume);
      configChange = true;
      break;
      break;
    case BUTTON12:     // Alarm Menu SAVE & EXIT
      //playGobbleSound(); // Play button confirmation sound
      playGobble = true;
      PACMAN_PRINTLN(F("Save & Exit"));
      results = true;
      break;
    default:
      break;
  }
  return results;
}

////////////////////////////////////////////////////////////
//
//  Display Alarm Settings
//
void displayAlarmSetting()
{
  // Refresh the alarm settings
  // Erase Current Alarm Time
  // Push Text Attributes on attribute stack
  pushTA();
  tft.fillRect(60, 50, 200, 50, TFT_BLACK);
  tft.setTextSize(4);      // Text size multiplier
  if (clockConfig.alarmHour < 13)
  {
    // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate
    if (clockConfig.alarmHour > 9)
    {
      tft.drawNumber(clockConfig.alarmHour, 90, 55);   // If >= 10 just print hour
    }
    else
    {
      tft.drawNumber(clockConfig.alarmHour, 110, 55);   // If >= 10 just print hour
    }
  }
  else
  {
    if ((clockConfig.alarmHour - 12) > 9)
    {
      tft.drawNumber((clockConfig.alarmHour - 12), 90, 55); // If >= 10 just print hour
    }
    else
    {
      tft.drawNumber((clockConfig.alarmHour - 12), 105, 55); // If >= 10 just print hour
    }
  }
  // Draw am/pm label
  if ((clockConfig.alarmHour < 12) || (clockConfig.alarmHour == 24 )) {
    tft.setTextSize(3);        // Text size multiplier
    tft.drawString("am", 260, 58);
    tft.setTextSize(4);        // Text size multiplier
  }
  else
  {
    tft.setTextSize(3);        // Text size multiplier
    tft.drawString("pm", 260, 58);
    tft.setTextSize(4);        // Text size multiplier
  }
  tft.drawString(":", 160, 55);
  if (clockConfig.alarmMinute >= 10)
  {
    // Annoyingly if number less than 10 doesnt print on RHS and misses zero so need to compensate
    tft.drawNumber(clockConfig.alarmMinute, 205, 55);   // If >= 10 just print minute
  }
  else
  {
    tft.drawString("0", 205, 55);
    tft.drawNumber(clockConfig.alarmMinute, 233, 55);
  }
  // Draw Alarm Status
  tft.setTextSize(2);        // Text size multiplier
  
  drawSwitchV( BUTTON1, clockConfig.alarmStatus);
  
  drawBtn(7, TFT_WHITE, "Volume", "UP");
  drawBtn(10, TFT_WHITE, "Volume", "DOWN");
  drawVolumeLevel(clockConfig.alarmVolume, true);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Alarm Set buttons
  tft.setTextSize(3);        // Text size multiplier
  tft.drawString(" +      +", 107, 20);
  tft.drawString(" -      -", 107, 100);
  // Setup the Alarm Save and Exit options
  // Pop Text Attributes from attribute stack
  popTA();
  drawBtn(12, TFT_WHITE, "Exit &", "Save");
}

////////////////////////////////////////////////////////////
//
//  Display WiFi Menu
//
void displayWiFiMenu(void)
{
  menuState = WIFI_MENU;
  drawFullScreenOutline();
  // Push Text Attributes on attribute stack
  pushTA();
  setGfxFont(FSS12);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString("WiFi Settings", pgCenter, pgLine2, GFXFF);
  drawSwitchH( BUTTON5, clockConfig.wifiEnabled);
  if(clockConfig.wifiEnabled)
  {
    drawBtn(11, TFT_WHITE, "WiFi", "Config");
  }
  //drawBtn(12, TFT_WHITE, "Exit", "");
  if (changedWiFiEnable == true)
    drawBtn(12, TFT_WHITE, "Save", "& Exit");
  else
    drawBtn(12, TFT_WHITE, "Exit", "");
    // Pop Text Attributes from attribute stack
  popTA();
}


////////////////////////////////////////////////////////////
//
//  Process WiFi Menu (screen touches)
//
bool processWiFiMenu(void)
{
  bool results = false;
  if (touchData.button == BUTTON12)        // Save & Exit
  {
    if(changedWiFiEnable)
    {
      // update clockConfig in SPIFF
      wrtFile(fnClockConfig, (char *)&clockConfig, sizeof(clockConfig));
      restart();
    }
    results = true;
  }
  else if (touchData.button == BUTTON6)   // Enable WiFi (ON)
  {
    if(clockConfig.wifiEnabled == false)
    {
      clockConfig.wifiEnabled = true;
      changedWiFiEnable = true;
      displayWiFiMenu();
      //results = true;
    }
  }
  else if (touchData.button == BUTTON4)   // Disable WiFi (OFF)
  {
    if(clockConfig.wifiEnabled == true)
    {
      clockConfig.wifiEnabled = false;
      changedWiFiEnable = true;
      displayWiFiMenu();
      //results = true;
    }
  }
  else if (touchData.button == BUTTON5)   // Toggle WiFi Enabled
  {
    if(clockConfig.wifiEnabled == true)
    {
      clockConfig.wifiEnabled = false;
    } else clockConfig.wifiEnabled = true;
    changedWiFiEnable = true;
    displayWiFiMenu();
  }
  else if(touchData.button == BUTTON11)   // WiFi Config
  {
    displayWiFiConfigMenu();
  }
  return results;
}

 
////////////////////////////////////////////////////////////
//
//  Display WiFi Configuration Menu
//
void displayWiFiConfigMenu(void)
{
  menuState = WIFI_CONFIG_MENU;
  drawFullScreenOutline();
  // Push Text Attributes on attribute stack
  pushTA();
  drawMenu();
  setGfxFont(FSS12);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString("WiFi Settings", puFrameCenter, pgLine2, GFXFF);
  tft.setTextDatum(TL_DATUM);        // Left Justify Text (default)
  setGfxFont(FSS9);
  tft.drawString(WiFi.macAddress(), puFrameLeft + 20, pgLine3 + 10, GFXFF);;
  tft.drawFastHLine(puFrameLeft, pgLine4 + 10, puFrameWidth, TFT_GREEN);  //3
  setGfxFont(FSS9);
  tft.drawString("SSID: ", puFrameLeft + 10, pgLine5, GFXFF);    //4
  displayWiFiSSID();
  tft.drawFastHLine(puFrameLeft, pgLine7 + 10, puFrameWidth, TFT_GREEN);
  tft.drawString("Password: ", puFrameLeft + 10, pgLine8, GFXFF);
  displayWiFiPW();
  if ((changedSSID == true) || (changedPW == true))
    drawBtn(12, TFT_WHITE, "Save", "& Exit");
  else
    drawBtn(12, TFT_WHITE, "Exit", "");
  drawBtn(3, TFT_WHITE, "Cancel", "");
  // Pop Text Attributes from attribute stack
  popTA();
}

////////////////////////////////////////////////////////////
//
//  Process WiFi Configuration Menu (screen touches)
//
bool processWiFiConfigMenu(void)
{
  bool results = false;
  if (touchData.button == BUTTON12)        // Save & Exit
  {
    if (changedSSID == true)       // if changed/selected SSID
    {
      strncpy(WiFiConfig.ssID, tempSSID, 32);
    }
    if (changedPW == true)       // if changed/selected Password
    {
      strncpy(WiFiConfig.passWord, tempPW, 32);
    }
    if ((changedSSID == true) || (changedPW == true))
    {
      // Update WiFi configuration in SPIFFS.
      wrtFile(fnWifiConfig, (char *)&WiFiConfig, sizeof(WiFiConfig));
      // Reset ESP32 to restart WiFi
      restart();
    }
    results = true;
  }
  else if (touchData.button == BUTTON3)   // Cancel
  {
    results = true;
  }
  else if (touchData.x < 210)
  {
    if ((touchData.y >= pgLine6) && (touchData.y < pgLine7))
    {
      menuState = SSID_MENU;
      displaySsidMenu();
    }
    else if ((touchData.y >= pgLine10) && (touchData.y < pgLine11))
    {
      // Touched Password
      pwKeypad = LC_KEYPAD;
      keypadBtn3UC = false;
      menuState = PW_MENU;
      //passSelected = false;
      changedPW = false;
      displayPwMenu();
    }
  }
  return results;
}

///////////////////////////////////////////////////////////
//
//  Display WiFi SSID in WiFi Menu
//
//void displayWiFiSSID(int font_color, int background_color)
void displayWiFiSSID(void)
{
  // Push Text Attributes on attribute stack
  pushTA();
  eraseLine(pgLine6);
  if (changedSSID)
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setGfxFont(FSS12);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString(tempSSID, puFrameCenter, pgLine6, GFXFF);
  // Pop Text Attributes from attribute stack
  popTA();
}

/////////////////////////////////////////////////////////
//
//  Display WiFi Password in WiFi Menu
//
//void displayWiFiPW(int font_color, int background_color)
void displayWiFiPW(void)
{
  // Push Text Attributes on attribute stack
  pushTA();
  eraseLine(pgLine10);
  if (changedPW)
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  //tft.setTextColor(font_color, background_color);
  setGfxFont(FSS12);
  tft.setTextDatum(TC_DATUM);         // Center Text
  if (changedPW) tft.drawString(tempPW, puFrameCenter, pgLine10, GFXFF);
  else
    tft.drawString("*********", puFrameCenter, pgLine10, GFXFF);
  // pop Text Attributes from attribute stack
  popTA();
}

/////////////////////////////////////////////////////////////
//
//  Display WiFi SSID Menu (available networks)
//
void displaySsidMenu(void)
{
  ssidSelected = false;
  drawBtn(6, TFT_ORANGE, "Scanning", "");
  FillSsidStruct();
  displayWifiSSIDtbl();
}

///////////////////////////////////////////////////////////
//
//  Process SSID Menu (screen touches)
//    returns:  TRUE if "Cancel" or "Save" button touched
//              indicating exit of SSID menu.
//              FALSE if continuing in SSID Menu.
//
bool processSsidMenu(void)
{
  bool results = false;
  if (touchData.button == BUTTON3)            // Cancel
  {
    ssidSelected = false;
    changedSSID = false;
    results = true;
  }
  else if (touchData.button == BUTTON6)       // Scan
  {
    drawBtn(6, TFT_ORANGE, "Scanning", "");
    ssidSelected = false;
    changedSSID = false;
    FillSsidStruct();
    displayWifiSSIDtbl();
  }
  else if (touchData.button == BUTTON12)       // Save & Exit
  {
    if (changedSSID)
    {
      strncpy(tempSSID, WifiNetworks.Tbl[ssidSelectIndex].ssid, 32);
    }
    results = true;
  }
  else if (touchData.x < 210)
  {
    if ((touchData.row >= 3) && (touchData.row <= 10))
    {
      ssidSelectIndex = touchData.row - 3;
      ssidSelected = true;
      changedSSID = true;
      displayWifiSSIDtbl();
    }
  }
  return results;
}

////////////////////////////////////////////////////////////
//
//  Display WiFi Password Menu
//
void displayPwMenu(void)
{
  switch (pwKeypad)
  {
    case LC_KEYPAD:
      drawKeyboard(LC_KEYPAD);
      drawBtn(3, TFT_WHITE, "ABC...", "");
      keypadBtn3UC = true;
      break;
    case UC_KEYPAD:
      drawKeyboard(UC_KEYPAD);
      drawBtn(3, TFT_WHITE, "abc...", "");
      keypadBtn3UC = false;
      break;
    case NUMBER_KEYPAD:
      drawKeyboard(NUMBER_KEYPAD);
      break;
    default:
      break;
  }
  drawBtn(6, TFT_WHITE, "123...", "");
  drawBtn(9, TFT_WHITE, "Backspace", "");
  drawBtn(12, TFT_WHITE, "Done", "");
}

//////////////////////////////////////////////////////////
//
//  Process WiFi Password (screen touches)
//
bool processPwMenu(void)
{
  bool results = false;
  if (touchData.button == BUTTON3)            // select upper or lower case keypad
  {
    if (keypadBtn3UC) pwKeypad = UC_KEYPAD;
    else pwKeypad = LC_KEYPAD;
  }
  else if (touchData.button == BUTTON6)       // select number keypad
  {
    pwKeypad = NUMBER_KEYPAD;
  }
  else if (touchData.button == BUTTON9)       // Backspace
    backspaceKB();
  else if (touchData.button == BUTTON12)       // Done
  {
    if (inputStrPtr)  // was anything typed ?
    {
      changedPW = true;
      strncpy(tempPW, inputStr, 32);
      //passSelected = true;
    }
    results = true;
  }
  else getKBchar(pwKeypad);

  displayPwMenu();
  return results;
}

/////////////////////////////////////////////////////////
//
//  Draw Keyboard (used for entering WiFi Password)
//
void drawKeyboard( char startChar)
{
  drawMenu();
  tft.drawFastHLine(puFrameLeft, pgLine3 + 10, puFrameWidth, TFT_GREEN);
  drawKbStr();
  char buf[40];
  int xpos = 20;
  int ypos = pgLine4 - 3;
  int xwidth = 30;
  int yheight = 16;
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  // Draw Buttons (four per row, eight rows)
  for ( int r = 0; r < 8; r++)    // rows
  {
    for ( int c = 0; c < 4; c++) // columns
    {
      tft.drawRoundRect( xpos + (c * 50), ypos + (r * 20), xwidth + 2, yheight + 2, 9, TFT_WHITE);
      tft.fillRoundRect( xpos + (c * 50) + 1, ypos + (r * 20) + 1, xwidth, yheight, 9, TFT_BLUE);
      buf[0] = startChar++ ;   /// + c + (r * 0x4));
      buf[1] = 0;
      tft.drawString(buf, xpos + (c * 50) + 13, ypos + (r * 20) + 1, 2);
    }
  }
}

//////////////////////////////////////////////////////////////////
//
//  Draw Keyboard input string (used for entering WiFi password)
//
void drawKbStr(void)
{
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString(kbTitle, 120, pgLine1, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(FSS9);
  tft.drawString( inputStr, 120, pgLine2, GFXFF);
  tft.setTextDatum(TL_DATUM);         // Left Justify Text
}

////////////////////////////////////////////////////
//
//  Get Keyboard Character (screen touches)
//
void getKBchar(char basechar)
{
  char txt = basechar;
  int row = touchData.row;
  int column = touchData.column;
  if ((row >= 3) && (row <= 10))
  {
    txt += ((row - 3) * 4) + column;
    if (inputStrPtr < INPUT_STR_SZ)
    {
      inputStr[inputStrPtr++] = txt;
      inputStr[inputStrPtr] = 0;
      drawKbStr();
    }
  }
}

//////////////////////////////////////////////////////////
//
//  Back Space Keyboard entry
//
void backspaceKB(void)
{
  if (inputStrPtr > 0)
  {
    inputStrPtr--;
    inputStr[inputStrPtr] = 0;
    eraseLine(pgLine2);
    drawKbStr();
  }
}

////////////////////////////////////////////////////////////
//
//  Display Time Zone Menu
//
void displayTzMenu(void)
{
  menuState = TZ_MENU;
  drawFullScreenOutline();
  //
  //  Display Time Zone Setting .
  //
  // Push Text Attributes on attribute stack
  pushTA();
  drawMenu();
  setGfxFont(FSS12);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString("Time Zone", puFrameCenter, pgLine2, GFXFF);
  tft.setTextDatum(TL_DATUM);        // Left Justify Text (default)

  tft.drawFastHLine(puFrameLeft, pgLine3 + 10, puFrameWidth, TFT_GREEN);
  setGfxFont(FSS9);
  tft.drawString("Region: ", puFrameLeft + 10, pgLine4, GFXFF);
  displayTzRegion();

  tft.drawFastHLine(puFrameLeft, pgLine7 + 10, puFrameWidth, TFT_GREEN);
  tft.drawString("City: ", puFrameLeft + 10, pgLine8, GFXFF);
  displayTzCity();

  if ((regionSelected) || (citySelected))
    drawBtn(12, TFT_WHITE, "Exit", "& Save");
  else
    drawBtn(12, TFT_WHITE, "Exit", "");
  drawBtn(3, TFT_WHITE, "Cancel", "");
  // Pop Text Attributes from attribute stack
  popTA();
}

////////////////////////////////////////////////////////
//
//  Process Time Zone menu (screen touches)
//
bool processTzMenu(void)
{
  bool results = false;
  if (touchData.button == BUTTON3)     // Cancel
  {
    regionSelected = false;
    citySelected = false;
    results =  true;
  }
  else if (touchData.button == BUTTON12)       // Done
  {
    if ((regionSelected) || (citySelected))
    {
      tzConfig.regionIndx = tzTemp.regionIndx;
      tzConfig.cityIndx = tzTemp.cityIndx;
      strncpy(tzConfig.tzInfo, tzTemp.tzInfo, 48);
      wrtFile(fnTimeZoneConfig, (char *)&tzConfig, sizeof(tzConfig));
      setTZ();
      printTzTemp();
    }
    results = true;
  }
  else if (touchData.x < 210)
  {
    if ((touchData.y >= pgLine6) && (touchData.y < pgLine7))
    {
      // Touched "Region"
      menuState = REGION_MENU;
      regionSelected = false;
      startRegion = 0;
      displayRegionMenu();
    }
    else if ((touchData.y >= pgLine10) && (touchData.y < pgLine11))
    {
      // Touched "City"
      menuState = CITY_MENU;
      citySelected = false;
      startCity = 0;
      displayCityMenu();
    }
  }
  return results;
}

/////////////////////////////////////////////////////////
//
//  Display Time Zone Region in Time Zone Menu
//
//
void displayTzRegion(void)
{
  // Push Text Attributes on attribute stack
  pushTA();
  eraseLine(pgLine6);
  if (regionSelected)
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setGfxFont(FSS12);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString(regionName[tzTemp.regionIndx], puFrameCenter, pgLine6, GFXFF);
  // Pop Text Attributes from attribute stack
  popTA();
}

////////////////////////////////////////////////////////////
//
//  Process Region (selection) Menu (screen touches)
//
bool processRegionMenu(void)
{
  bool results = false;
  if (touchData.button == BUTTON3)     // Cancel
  {
    results =  true;
  }
  else if (touchData.button == BUTTON12)     // Done
  {
    if (regionSelected)
    {
      Serial.printf("changedRegionIndx = %d\n\r", changedRegionIndx);
      tzTemp.regionIndx = changedRegionIndx;
      tzTemp.cityIndx = 0;
    }
    results =  true;
  }
  else if (touchData.button == BUTTON6)   //  Scroll Up
  {
    if (startRegion == 0)
    {
      regionSelected = false;
      startRegion += 8;
      displayRegionMenu();
    }
  }
  else if (touchData.button == BUTTON9)   // Scroll Down
  {
    if (startRegion != 0)
    {
      regionSelected = false;
      startRegion -= 8;
      displayRegionMenu();
    }
  }
  else if (touchData.x < 210)
  {
    if ((touchData.row >= 3) && (touchData.row <= 10))
    {
      regionSelectIndex = touchData.row - 3;
      regionSelected = true;
      displayRegionMenu();
    }
  }
  return results;
}

/////////////////////////////////////////////////////////
//
//  Display Time Zone City in Time Zone Menu
//
void displayTzCity(void)
{
  loadTzCity();
  // Push Text Attributes on attribute stack
  pushTA();
  eraseLine(pgLine10);
  if (citySelected)
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setGfxFont(FSS12);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString(city.cityName, puFrameCenter, pgLine10, GFXFF);
  // pop Text Attributes from attribute stack
  popTA();
}

////////////////////////////////////////////////////////////
//
//  Process City (selection) Menu (screen touches)
//
bool processCityMenu(void)
{
  bool results = false;
  if (touchData.button == BUTTON3)     // Cancel
  {
    results =  true;
  }
  else if (touchData.button == BUTTON12)     // Done
  {
    if (citySelected)
    {
      tzTemp.cityIndx = changedCityIndx;
      strncpy(tzTemp.tzInfo, changedTzInfo, 48);
    }
    results =  true;
  }
  else if (touchData.button == BUTTON6)   //  Scroll Up
  {
    if (startCity < (cityCount - 8))
    {
      citySelected = false;
      startCity += 8;
      displayCityMenu();
    }
  }
  else if (touchData.button == BUTTON9)   // Scroll Down
  {
    if (startCity >= 8)
    {
      citySelected = false;
      startCity -= 8;
      displayCityMenu();
    }
  }
  else if (touchData.x < 210)
  {
    if ((touchData.row >= 3) && (touchData.row <= 10))
    {
      citySelectIndex = touchData.row - 3;
      if ((startCity + citySelectIndex) < cityCount)
      {
        Serial.printf("index of City selected = %d\n\r", startCity + citySelectIndex);
        citySelected = true;
        tzTemp.cityIndx = startCity + citySelectIndex;
        printCityInfo(startCity + citySelectIndex);
      }
      else
        Serial.println("Selected City out of range");
      displayCityMenu();
    }
  }
  return results;
}

//////////////////////////////////////////////////////////
//
//  Erase line (row)  (2 - 11)
//
void eraseLine(int lineNo)
{
  tft.fillRect(puFrameLeft + 2, lineNo, puFrameWidth - 4, pgLineHeight, TFT_BLACK);
}

/////////////////////////////////////////////////
//
//    drawMenu() - generic draw for any setup Menu
//
void drawMenu(void)
{
  // erase buttons 6 & 9
  tft.fillRoundRect(220 , 68 , B_WIDTH  , B_HEIGHT  , B_RADIUS , TFT_BLACK);   // button #6
  tft.fillRoundRect(220 , 127 , B_WIDTH  , B_HEIGHT  , B_RADIUS , TFT_BLACK);  // button #9
  //
  tft.fillRect(puFrameLeft, puFrameTop, puFrameWidth, puFrameHeight, TFT_BLACK);
  tft.drawRoundRect(puFrameLeft, puFrameTop - 1, puFrameWidth, puFrameHeight + 2, 9, TFT_GREEN);
  tft.drawRoundRect(puFrameLeft - 1, puFrameTop - 2, puFrameWidth + 2, puFrameHeight + 4, 9, TFT_GREEN);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}


/////////////////////////////////////////////////////////
//
//  Display Region Menu
//
void displayRegionMenu(void)
{
  // Push Text Attributes on attribute stack
  pushTA();
  drawMenu();
  tft.drawFastHLine(puFrameLeft, pgLine3 + 10, puFrameWidth, TFT_GREEN);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setGfxFont(FSS9);
  tft.drawString("Select a Region", puFrameCenter, pgLine2, GFXFF);
  // Draw Regions
  tft.setTextDatum(TC_DATUM);         // Center Text
  for (int i = 0; i < 8; i++)
  {
    if ((startRegion + i) < REGION_COUNT)
    {
      if ((regionSelected) && ( i == regionSelectIndex))
      {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        changedRegionIndx = startRegion + i;
      }
      else
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      //if ((startRegion + i) < REGION_COUNT)
      tft.drawString(String(regionName[i + startRegion]), puFrameCenter, pgLine4 + (i * pgLineHeight), 2);  // Start with pgLine4
    }
  }
  if ((REGION_COUNT - startRegion) > 8) drawBtn(6, TFT_WHITE, "Scroll", "Up");
  if (startRegion > 0) drawBtn(9, TFT_WHITE, "Scroll", "Down");
  drawBtn(12, TFT_WHITE, "Exit", "");
  // Pop Text Attributes from attribute stack
  popTA();
}

////////////////////////////////////////////////////////
//
//  Display City Menu
//
void displayCityMenu(void)
{
  char fnBuf[32];
  int recordCnt;
  int regionIndx = tzTemp.regionIndx;
  // Push Text Attributes on attribute stack
  pushTA();
  drawMenu();
  setGfxFont(FSS12);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString("Select a City", puFrameCenter, pgLine2, GFXFF);
  tft.setTextDatum(TL_DATUM);        // Left Justify Text (default)

  tft.drawFastHLine(puFrameLeft, pgLine3 + 10, puFrameWidth, TFT_GREEN);
  setGfxFont(FSS9);
  Serial.printf("-------- %s --------\n\r", regionName[regionIndx]);
  sprintf(fnBuf, "/%s.txt", regionName[regionIndx]);
  fTXT = SPIFFS.open(fnBuf, "r");

  if (!fTXT) {
    Serial.println(F("loadTzCity() Failed to open file"));
    Serial.println(fnBuf);
    while (1);  // hang here
  }
  //
  //
  recordCnt = cityCount = indexFile();
  if (recordCnt != 0)
  {
    for (int c = 0; c < 8; c++)
    {
      if ((c + startCity) < cityCount)
      {
        fillCityStruct(c + startCity);
        tft.setTextDatum(TC_DATUM);         // Center Text
        if ((citySelected) && ( c == citySelectIndex))
        {
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          strncpy(changedTzInfo, city.tzInfo, 48);
          changedCityIndx = startCity + c;
        }
        else
          tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(city.cityName), puFrameCenter, pgLine4 + (c * pgLineHeight), 2);  // Start with pgLine4
      }
    }
  }
  else
  {
    Serial.println(F("displayCityMenu() no cities ? "));
  }
  if ((cityCount - startCity) > 8) drawBtn(6, TFT_WHITE, "Scroll", "Up");
  if (startCity != 0) drawBtn(9, TFT_WHITE, "Scroll", "Down");
  Serial.printf("startCity = %d\n\r", startCity);
  fTXT.close();
  // Pop Text Attributes from attribute stack
  popTA();
}


////////////////////////////////////////////////////////////
//
//  Draw Menu Outline (full screen)
//    Displays initial blank menu screen 
//
//
void drawFullScreenOutline(void)
{
  // Push Text Attributes on attribute stack
  pushTA();
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);        // Text size multiplier
  tft.fillScreen(TFT_BLACK);

  // ---------------- Outside wall
  tft.drawRoundRect(0  , 0 , 319 , 239 , 2 , TFT_YELLOW);
  tft.drawRoundRect(2  , 2 , 315 , 235 , 2 , TFT_YELLOW);
  // Pop Text Attributes from attribute stack
  popTA();
}

//////////////////////////////////////////////////////////
//
//  Display WiFi SSID Table (available networks)
//
void displayWifiSSIDtbl(void)
{
  // Push Text Attributes on attribute stack
  pushTA();
  drawMenu();
  tft.drawFastHLine(puFrameLeft, pgLine3 + 10, puFrameWidth, TFT_GREEN);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  setGfxFont(FSS9);
  tft.drawString("Available Networks", puFrameCenter, pgLine2, GFXFF);
  // Draw SSIDs
  tft.setTextDatum(TC_DATUM);         // Center Text
  for (int i = 0; i < WifiNetworks.count; i++)
  {
    if ((ssidSelected) && ( i == ssidSelectIndex))
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
    else
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(WifiNetworks.Tbl[i].ssid), puFrameCenter, pgLine4 + (i * pgLineHeight), 2);  // Start with pgLine4
  }
  drawBtn(6, TFT_WHITE, "Scan", "");
  if (ssidSelected == true)
    drawBtn(12, TFT_WHITE, "Save", "& Exit");
  else
    drawBtn(12, TFT_WHITE, "Exit", "");

  // Pop Text Attributes from attribute stack
  popTA();
}

//////////////////////////////////////////////////////////
//
//  Load city structure
//      using tzTemp.regionIndx and tzTemp.cityIndx
//
//
void loadTzCity(void)
{
  char fnBuf[32];
  int recordCnt;
  int regionIndx = tzTemp.regionIndx;
  // Push Text Attributes on attribute stack
  pushTA();
  Serial.printf("!!----- %s ------!!\n\r", regionName[regionIndx]);
  sprintf(fnBuf, "/%s.txt", regionName[regionIndx]);
  Serial.printf("Opening file %s\n\r", fnBuf);
  fTXT = SPIFFS.open(fnBuf, "r");

  if (!fTXT) {
    Serial.println(F("loadTzCity() Failed to open file"));
    while (1);  // hang here
  }
  //
  //  Fill city struct from file
  //
  recordCnt = indexFile();
  if (recordCnt != 0)
  {
    fillCityStruct(tzTemp.cityIndx);
  }
  fTXT.close();
  // Pop Text Attributes from attribute stack
  popTA();
}


//////////////////////////////////////////////////////
//
//  get Record at offset 'recordIndx'
//    store read data in record.rData and length in record.rSize (record structure).
//    The file at handle fTXT must already be opened before calling this function.
//
bool getRecord(int recordIndx)
{
  bool result = false;
  bool done = false;
  int rdLength = 0;
  if (recordIndxTbl[recordIndx].recordSize == 0) return false;
  while (!done)
  {
    if (recordIndxTbl[recordIndx].recordSize == 0)
    {
      done = true;
    }
    else
    {
      Serial.printf(" %d  Start = %d, Length = %d\n\r", recordIndx,
                    recordIndxTbl[recordIndx].recordStart, recordIndxTbl[recordIndx].recordSize);
      fTXT.seek(recordIndxTbl[recordIndx].recordStart, SeekSet);
      record.rSize = recordIndxTbl[recordIndx].recordSize;
      rdLength = fTXT.readBytes( record.rData, record.rSize);
      if (rdLength != record.rSize)
      {
        Serial.printf("Read ERROR length Exp : %d  Act : %d\n\r", recordIndxTbl[recordIndx].recordSize, rdLength);
        while (1);
      }
      else
      {
        record.rData[record.rSize] = 0;
        done = true;
        result = true;
      }
    }
  }
  return result;
}

//////////////////////////////////////////////////////////////////////
//
//  Fill City Data Struct (global structure) from file handle (fTXT)
//    Must have first called indexFile()
//    Uses file handle fTXT. File must already be opened.
//
void fillCityStruct(int indx)
{
  // get record at indx ( must have first called indexFile()
  if (getRecord(indx) == true)
  {
    int subIndx = String(record.rData).indexOf(',');
    int z = 0;
    for ( z = 0; z < subIndx; z++)
    {
      city.cityName[z] = record.rData[z];
    }
    city.cityName[z] = 0;
    int x = 0;
    for ( x = 0; (x + subIndx + 1) < record.rSize; x++)
    {
      city.tzInfo[x] = record.rData[x + subIndx + 1];
    }
    city.tzInfo[x] = 0;
  }
  else
  {
    city.tzInfo[0] = 0;
    city.cityName[0] = 0;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
//
//  Populate Record Index Table (which is global). (text files with LF,CR,0 record delimiters)
//      For each delimited string (record) in the specified file, the record index table
//      will contain the record location (byte count from the start of the file)
//      and the length of the string (record).
//
//    Requirements: file handle fTXT - must be opened prior to callind this function.
//                  Populates recordIndxTbl[] (global structure)
//
//    Returns: Record count
//
int indexFile(void)
{
  int recordCount = 0;
  int currentRecordIndx = 0;
  recordIndxTbl[0].recordSize = 0;
  //  File fTXT;
  bool recordDone = false;
  bool endOfFile = false;
  int rdLength = 0;
  int recordLength = 0;
  char cBuf[4];
  Serial.println(F("indexFile()"));
  fTXT.seek(0, SeekSet);          // Start from begining of file
  while (!endOfFile)          // end of file ?
  {
    recordLength = 0;
    recordDone = false;
    while (!recordDone)
    {
      if (recordLength == 0)      // possibly begining of record.
        recordIndxTbl[currentRecordIndx].recordStart = fTXT.position();
      // read a byte from file
      rdLength = fTXT.readBytes(cBuf, 1);
      if (rdLength != 0)
      {
        // Check for ASCII printable character,
        // non-printable are considered record delimiter(s)
        //
        if (cBuf[0] >= ' ')     // ASCII printable character
        {
          recordLength++;
        }
        // else must be record delimiter(s)
        // if consective delimiters (LF & CR) don't count as
        //  zero length record.
        else if (recordLength != 0) // don't count zero length records
        {
          recordCount++;
          recordIndxTbl[currentRecordIndx].recordSize = recordLength;
          currentRecordIndx++;
          recordDone = true;
        }
      }
      else      /// nothing left to read (end of file)
      {
        recordDone = true;    // needed to end (skip last delimiters)
        endOfFile = true;
      }
    }     // recordDone ?
  }       // End Of File ?
  // Mark the end of record index table entries
  recordIndxTbl[currentRecordIndx].recordSize = 0;
  recordIndxTbl[currentRecordIndx].recordStart = 0;
  cityCount = recordCount;
  return recordCount;
}

////////////////////////////////////////////////////////////
//
//  For debugging
//
void printCityInfo(int cityIndx)
{
  char fnBuf[32];
  sprintf(fnBuf, "/%s.txt", regionName[tzTemp.regionIndx]);
  fTXT = SPIFFS.open(fnBuf, "r");

  if (!fTXT) {
    Serial.println(F("loadTzCity() Failed to open file"));
    while (1);  // hang here
  }
  fillCityStruct(cityIndx);
  Serial.printf(" %s   %s\n\r", city.cityName, city.tzInfo);
  fTXT.close();
}

////////////////////////////////////////////////////
//
//
//
void printTzTemp(void)
{
  Serial.printf("tzTemp region %d : city %d : tzInfo %s\n\r", tzTemp.regionIndx, tzTemp.cityIndx, tzTemp.tzInfo);
}



/////////////////////////////////////////////////////////////
//
//  Configure the Time Zone - reads time zone configuration from SPIFFS.
//
void configTZ(void)
{
  if (!existsFile(fnTimeZoneConfig))
  {
    CONFIG_PRINTLN("Creating TimeZone configuration file");
    // Americas, Los Angeles
    tzConfig.regionIndx = 1;
    tzConfig.cityIndx = 82;
    sprintf(tzConfig.tzInfo, " %s", "PST8PDT, M3.2.0, M11.1.0");
    wrtFile(fnTimeZoneConfig, (char *)&tzConfig, sizeof(tzConfig));
  }
  else
  {
    CONFIG_PRINTLN("Reading TimeZone configuration file");
    rdFile(fnTimeZoneConfig, (char *)&tzConfig, sizeof(tzConfig));
  }
  setTZ();
}

///////////////////////////////////////////////////////////
//
//
//
void setTZ(void)
{
  configTime(0, 0, ntpServer3);
  setenv("TZ", tzConfig.tzInfo, 1);
  tzset();
}

////////////////////////////////////////////////////
//
//
//
void displayClock12_24(void)
{
  if (clockConfig.clock24) drawBtn(10, TFT_WHITE, "24 Hour", "");
  else drawBtn(10, TFT_WHITE, "12 Hour", "");
}

////////////////////////////////////////////////////
//
//
//
void restart(void)
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);        // Text size multiplier
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(TC_DATUM);         // Center Text
  tft.drawString("- Restarting -", 160, 100);
  delay(1000);
  ESP.restart();
}
