///////////////////////////////////////////////
//
// used by setSyncProvider()
//
time_t getDS3231_Time()
{
  Serial.print("getDS3231_Time: ");
  Serial.print("DS3231 Epoch Time: ");
  Serial.println(ds3231.now().unixtime());
  return ds3231.now().unixtime(); 
}

/////////////////////////////////////////////////////
//
//  Sync ESP32 RTC with NTP or DS3231 time.
//  If both NTP and DS3231 then update DS3231 with NTP time.
//
long syncTime(void)
{
  long actualTime = 0;
  Serial.println("--- syncTime ---");
  if(wifiConnectFlag)
  {
    //timeClient.begin();
    //timeClient.setTimeOffset(timeOffset);
    //  while (!timeClient.update()) {
    if(timeClient.forceUpdate() == false)
    {
      // failed to get NTP time update
      Serial.println("Failed to get NTP time");
    }
    actualTime = timeClient.getEpochTime();
    if(DS3231ConnectFlag)  
    {
      // Update DS3231 with NTP time
      ds3231.adjust(DateTime(actualTime));
    }
    Serial.print("Internet Epoch Time: ");
    Serial.println(actualTime);
  }
  else if(DS3231ConnectFlag)
  {
    DateTime now = ds3231.now();
    actualTime = now.unixtime();
    Serial.print("RTC Epoch Time: ");
    Serial.println(now.unixtime());
  }
  setTime((time_t)actualTime);
  return (actualTime);
}

//////////////////////////////////////////////
//
//
//
bool DS3231_Init(void)
{
  //  Serial.println("Initializing RTC");
  if (!ds3231.begin(&myI2C)) 
  {
    Serial.println("Couldn't find RTC");
    delay(2000);
    return false;
  }
  Serial.println("Initializing DS3231");
  //pinMode(RTC_INT_PIN, INPUT_PULLUP);
  ds3231.disableAlarm(ALARM_1);      //ds3231.alarmInterrupt(ALARM_1, false);
  ds3231.disableAlarm(ALARM_2);      //ds3231.alarmInterrupt(ALARM_2, false);
  ds3231.writeSqwPinMode(DS3231_OFF );    //ds3231.squareWave(SQWAVE_NONE);
  ds3231.clearAlarm(ALARM_1);
  ds3231.clearAlarm(ALARM_2);
  // set alarm 1 to occur every second
  ds3231.setAlarm1( dt, DS3231_A1_PerSecond);
  // set alarm 2 to occur every minute
  ds3231.setAlarm2( dt, DS3231_A2_PerMinute);
  delay(1000);
  return true;
}

//
//
//
bool ntpConnect(void)
{
  if(wifiConnectFlag) 
  {
    Serial.println("ntpConnect()");
    delay(1000);
    timeClient.begin();
    timeClient.setTimeOffset(timeOffset);
    unsigned long currentMillis = millis();
    unsigned long previousMillis = currentMillis;
    // wait up to 5 seconds for NTP connection)
    while(timeClient.forceUpdate() == false)
    {
      currentMillis = millis();
      if (currentMillis - previousMillis >= 5000) 
      {
        Serial.println("Failed NTP connect.");
        return false;
      }
      delay(100);
    };
    Serial.println("NTP Connected");
    setTime((time_t)getNtpTime());
    //Serial.println("NTP sync provider");
    setSyncProvider(getNtpTime);
    setSyncInterval(60 * 10);  // 3600 seconds (1 hour)
    return(true);
  }
  else return(false);
}
