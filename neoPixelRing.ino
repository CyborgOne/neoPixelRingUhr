


/*
 * Pixel für aktuelle Zeiger-Markierung setzen
 */
void setClockPixel(){
    ring.setPixelColor(h-1>0?h-1:60+(h-1), c_hour1);
    ring.setPixelColor(h, c_hour);
    ring.setPixelColor(h+1<60?h+1:h+1-60, c_hour1);
   
    ring.setPixelColor(m, c_minute);
    
    ring.setPixelColor(s, c_second);
}


void messageAnimation() {
  for(int i=0;i<60;i++){
    if(i>0){
      ring.setPixelColor(i-1, 0, 0, 0);  
    }
    ring.setPixelColor(i, random(0,255), random(0,255), random(0,255));
    ring.show();   
    delay(20);
  }
  ring.setPixelColor(59, 0, 0, 0);  
  
}

/*
 * Alte Pixel entfernen
 */
void clearOldPixels(){
    ring.setPixelColor(lastHour, 0, 0, 0);
    ring.setPixelColor(lastHour+1, 0, 0, 0);
    ring.setPixelColor(lastHour-1, 0, 0, 0);

    ring.setPixelColor(lastMin, 0, 0, 0);
    
    ring.setPixelColor(lastSec, 0, 0, 0);
}


/*
 * Pixel für Uhr-Markierungen (5er Schritte) anzeigen
 * (weiß)
 */
void showClockMarker(){
    ring.setPixelColor(59, 10, 10, 10);
    ring.setPixelColor(0, 70, 70, 70);
    ring.setPixelColor(1, 10, 10, 10);
    
    for(int i=1;i<=12;i++){
      ring.setPixelColor((i*5), 5, 5, 5);
    }
}


/*
 * Farbwert ermitteln
 */
long getHourColor(){
  return c_hour;
}

long getMinuteColor(){
  return c_minute;
}

long getSecondColor(){
  return c_second;
}



/*
 * Helligkeit anhand des Sensorwerts regeln
 */
void refreshBrightness(){
    Serial.print("Neuer Sensorwert: ");
    Serial.println(sensorWert);
    
    bright = multiMap(sensorWert, in, out, 4);
    bright = (round(bright/30)*30)+15;
    ring.setBrightness(bright);
    Serial.print("Neue Helligkeit: ");
    Serial.println(bright);
}

/*
 * 
 */
byte darker(byte color, float lvl){
  float diffPercent = lvl / 100;
  byte ret = color - (color * diffPercent);

  return color>0?(ret>4?ret:4):0;
}


/*
 * Pixel Index ermitteln
 */
int getHourPixel(){
  int hTmp = now.hour();
  hTmp = hTmp==0?12:hTmp;

  byte hrStp = (now.minute()/12);
  
  return ((hTmp>12?hTmp-12:hTmp)*5)+hrStp;
}

int getMinutePixel(){
  return now.minute();
}

int getSecondPixel(){
  return now.second();
}


/*
 * Mehrfach Mapping
 */
int multiMap(int val, int* _in, int* _out, uint8_t size)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];

  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > _in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == _in[pos]) return _out[pos];

  // interpolate in the right segment for the rest
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}


/*
 * Farbring
 */
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}


