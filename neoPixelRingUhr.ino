#include <Adafruit_NeoPixel.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <RF24.h>
#include "RTClib.h"
#include <Wire.h>

/* ****************************************************
 *                 START CONFIGURATION
 **************************************************** */
// Data-Pin for Neo Pixel Ring
#define PIN 6
// Pin for Light-Sensor
#define LIGHT_PIN A3

// Pins for MD_MAX LED Matrix
#define  CLK_PIN   3
#define  DATA_PIN  4
#define  CS_PIN    5

#define NRF_CE_PIN  9
#define NRF_CSN_PIN 10

#define SET_CLOCK_HOUR_PIN 7
#define SET_CLOCK_MINUTE_PIN 8

// Count of 8x8 Blocks
#define  MAX_DEVICES 4
// Delay of the LED Matrix in milliseconds
#define SCROLL_DELAY  30  
// pixels between characters
#define  CHAR_SPACING  1 
// Maximum length of Characters for Textoutput on LED Matrix
#define  BUF_SIZE  50

// Settings for NeoPixel brightness
int out[]  = {1, 30,  100, 180,  255}; 
int in[]   = {0, 150, 350, 700, 1000};

// 3 Farbwerte (RGB) je Zeiger (Stunde, Minute, Sekunde)
byte colors[3][3] = {  {150,100,0},
                       {60,8,1},
                       {10,0,0}
                    };

// Helligkeit der ersten Dimm-Stufe
int colorDiffPercent1 = 55;

// pipes[0] = Klingel 
// pipes[1] = Freie Textnachricht
// ... weitere bisher ohne Verwendung
static const uint64_t pipes[6] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0B4LL, 0xF0F0F0F0A5LL, 0xF0F0F0F096LL};

static uint32_t keyDelay = 150;

/* ****************************************************
 *                 ENDE CONFIGURATION
 **************************************************** */


long c_hour   = -1;
long c_hour1   = -1;
long c_minute = -1;
long c_second = -1;
int lastSec = -1;
int lastMin = -1;
int lastHour = -1;
int sensorWert = -1;
int bright = -1;
int h = -1;
int m = -1;
int s = -1;

char messagePipe[10][BUF_SIZE];
char lastMessage[BUF_SIZE];

MD_MAX72XX mx = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN); 

char curMessage[BUF_SIZE];
char newMessage[BUF_SIZE];
bool newMessageAvailable = true;
bool reset = false;
static byte ndx = 0;
char endMarker = '\n';
char rc;
bool newData = false;
char receivedChars[BUF_SIZE];


Adafruit_NeoPixel ring = Adafruit_NeoPixel(60, PIN, NEO_GRBW + NEO_KHZ800);
RTC_DS1307 rtc;
DateTime now;

static uint32_t lastKeyPressedTime = 0;



void setup() {
  // Start Serial Port
  Serial.begin(57600);

  pinMode(SET_CLOCK_HOUR_PIN, INPUT);
  pinMode(SET_CLOCK_MINUTE_PIN, INPUT);

  pinMode(LIGHT_PIN, INPUT);

  for(int i=0;i<10;i++){
    strcpy(messagePipe[i],"");
  }

  // Init Led Matrix
  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);
  strcpy(curMessage, "Hello!        ");
  newMessage[0] = '\0';

  initRtc();

  initRf();

  ring.begin();
  
  messageAnimation();
  messageAnimation();
  messageAnimation();
  
  // Init NeoPixel Ring
  int r = 0;
  int g = 0;
  int b = 0;
  
  // Calculate Colors for hourhand
  c_hour    = ring.Color(colors[0][0],colors[0][1],colors[0][2]);
  c_hour1   = ring.Color(darker(colors[0][0],colorDiffPercent1),darker(colors[0][1],colorDiffPercent1),darker(colors[0][2],colorDiffPercent1));
  
  // Calculate Colors for minutehand
  c_minute  = ring.Color(colors[1][0],colors[1][1],colors[1][2]);
  
  // Calculate Colors for secondhand
  c_second  = ring.Color(colors[2][0],colors[2][1],colors[2][2]);

  // Show clock hands
  ring.show();
  
  delay(500);
}


void loop() {  
    // refresh current Time
    now = rtc.now();
    delay(20);
    
    sensorWert = analogRead(LIGHT_PIN);

    checkClockButtons();

    checkRf();

    h = getHourPixel();
    m = getMinutePixel();
    if(m==60){
      h=h+1;
      m=0;
    }
    s = getSecondPixel();

  // NeoPixel Ring nur aktualisieren,
  // wenn sich die Sekunden geändert haben
    if(lastSec != s){
      Serial.println(s);
      
      if(s == 0){
        char clockTime[10];
        Serial.println("wieder ne Minute rum");
        sprintf(clockTime, " %02i:%02i ", now.hour(), m);
        addNewTextToPipeline(clockTime); 
      }
      
      // remove old hands
      clearOldPixels();
      
      // show Markers for 5 Minute Steps
      showClockMarker();
      
      // recalculate Brightness dependent on Lightsensor
      refreshBrightness();
      
      // set new hand positions
      setClockPixel();
  
      // show new settings
      ring.show();
  
      // remember current time settings 
      // to clear hands on next loop
      lastSec  = s;
      lastMin  = m;
      lastHour = h;
    }
    
    // handle LED Matrix
    handleLedMatrix();

    // check for new Message by RF
    checkRf();
    
    // wait
    delay(10);
}

