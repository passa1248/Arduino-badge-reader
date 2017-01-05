
/*
 * script per l'invio di comandi AT al modulo esp8266 direttamente dal monitor seriale
 * la risposta dell'esp verrà a suo volta stampata a video,utile per fare test.
 */



#include <SoftwareSerial.h>


SoftwareSerial esp8266(8, 7); // RX, TX

void setup() {

//imposto il baudrate delle porte seriali
  Serial.begin(9600);
  esp8266.begin(9600);




}

void loop() { 
  
  if(Serial.available() > 0) {
   
    esp8266.write(Serial.read());
  }
  if(esp8266.available() > 0) {

    Serial.write(esp8266.read());
  }
}
