
uint8_t ClientNummer;
char signalValue[50] = "";
char signalOn[50] = "Klingel An";
 
void initRf(){
  radio.begin();
  delay(20);
  radio.setChannel(1);                // Funkkanal - Mögliche Werte: 0 - 127   
  radio.setAutoAck(0);    
  radio.setPALevel(RF24_PA_HIGH);     // Sendestärke darf die gesetzlichen Vorgaben des jeweiligen Landes nicht überschreiten! 
                                     // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm
  radio.setDataRate(RF24_1MBPS);

  radio.openReadingPipe(0,pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.openReadingPipe(2,pipes[2]);
  radio.openReadingPipe(3,pipes[3]);
  radio.openReadingPipe(4,pipes[4]);
  radio.openReadingPipe(5,pipes[5]);  
 
  radio.startListening();
}


void checkRf(){
  char got_message[50] = "";

  if (radio.available(&ClientNummer)){
    radio.read(&got_message, sizeof(got_message));

    Serial.println();
    Serial.print("ClientNummer: ");
    Serial.println(ClientNummer+1);
    Serial.print("Wert: ");
    Serial.println(got_message);
    
    if ((ClientNummer+1 == 1)) {       // Klingel
      if(strcmp(got_message, signalOn) == 0){
        radio.stopListening();
        addNewTextToPipeline("Klingelingeling... Es ist jemand am Eingang");
        delay(200);
        radio.startListening();
      } 
    } else if ((ClientNummer+1 == 2)) {
        radio.stopListening();
        addNewTextToPipeline(got_message);
        delay(200);
        radio.startListening();
    }
    
  }
}

