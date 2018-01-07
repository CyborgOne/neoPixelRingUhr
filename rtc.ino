

void initRtc(){
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

 void checkClockButtons(){
    if(digitalRead(SET_CLOCK_HOUR_PIN) == HIGH && lastKeyPressedTime+keyDelay<millis()){
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour()<23?now.hour()+1:0, now.minute(), now.second()));
      now = rtc.now();
      lastKeyPressedTime = millis();
    }
    
    if(digitalRead(SET_CLOCK_MINUTE_PIN) == HIGH && lastKeyPressedTime+keyDelay<millis()){
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute()<59?now.minute()+1:0, now.second()));
      now = rtc.now();
      lastKeyPressedTime = millis();
    }
 }

