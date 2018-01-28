/**
 * LED-Matrix verarbeiten
 */
void handleLedMatrix(){
  if(newMessageAvailable){
      scrollText();
    } else {
      if (strlen(messagePipe[0])>0) {
        messageAnimation();

        setNewText(messagePipe[0]);
        removeFirstEntryFromPipe();
        delay(10);
      } else {
        if(bright > 30){
          showClockTime();
        } else {
          mx.clear();
        }
      }
    }    
}

/**
 * Neuen Text in MessageQueue aufnehmen
 */
void addNewTextToPipeline(const char *newMessage){
  bool added = false;
  for(int index = 0; index < (sizeof(messagePipe)/BUF_SIZE)-1; index++){
      if(strstr(messagePipe[index], newMessage)!=NULL || strstr(lastMessage, newMessage)!=NULL){
        return;
      } else if (!added && strlen(messagePipe[index])==0 ) {
        Serial.print("Neu in Pipeline: ");
        Serial.println(newMessage);

        strcpy(messagePipe[index], newMessage);
        added = true;
      } 
  }
}


/**
 * Ersten Wert aus MessageQueue entfernen
 */
void removeFirstEntryFromPipe(){
  Serial.print("Entferne *");
  Serial.print(messagePipe[0]);
  Serial.println("* aus Pipeline");
  for(int index = 0; index < (sizeof(messagePipe)/BUF_SIZE)-1; index++){
    strcpy(messagePipe[index], messagePipe[index + 1]);
  }
  strcpy(messagePipe[sizeof(messagePipe)/BUF_SIZE-1], "");
}

 
/**
 * set new Text for scrolling Text
 */
void setNewText(const char *newMessage){
    Serial.print("Neue Nachricht: ");
    Serial.println(newMessage);
    strcpy(lastMessage, curMessage);
    sprintf(curMessage, "%s%s", newMessage, "          \0");
    newMessageAvailable = true;
    reset = true;
}


/**
 * show ClockTime in LED-Matrix (without scrolling)
 */
void showClockTime(){
  char clockTime[10];
  sprintf(clockTime, " %02i:%02i ", now.hour(), now.minute());
  printText(0, MAX_DEVICES-1, clockTime);
}


/**
 * Callback to handle scrolled out text
 */
void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col){
  #if PRINT_CALLBACK
    Serial.print("\n cb ");
    Serial.print(dev);
    Serial.print(' ');
    Serial.print(t);
    Serial.print(' ');
    Serial.println(col);
  #endif

  checkRf();
}


/**
 * Callback to handle text scrolling in
 */
uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t){
  static char   *p = curMessage;
  static uint8_t  state = 0;
  static uint8_t  curLen, showLen;
  static uint8_t  cBuf[8];
  uint8_t colData;
  if (reset){
      p = curMessage;
      reset = false;
  }
  // finite state machine to control what we do on the callback
  switch(state)
  {
    case 0: // Load the next character from the font table
      if (*p == '\0') {
       newMessageAvailable = false;
       strcpy(lastMessage, "");
       break;
      }
      showLen = mx.getChar(*p++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state++;

    case 1: // display the next part of the character
      colData = cBuf[curLen++];
      if (curLen == showLen)
      {
        showLen = CHAR_SPACING;
        curLen = 0;
        state = 2;
      }
      break;

    case 2: // display inter-character spacing (blank column)
      colData = 0;
      curLen++;
      if (curLen == showLen)
        state = 0;
      break;

    default:
      state = 0;
  }
  
  checkRf();

  return(colData);
}


/**
 * Callable method to scroll text
 */
void scrollText(void){
  static uint32_t prevTime = 0;
  if (millis()-prevTime >= SCROLL_DELAY){
    mx.transform(MD_MAX72XX::TSL);  // scroll along - the callback will load all the data
    prevTime = millis();      // starting point for next time
  }

  checkRf();
}


void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified. 
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t    curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)  
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3: // display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen) 
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }

    checkRf();

  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

