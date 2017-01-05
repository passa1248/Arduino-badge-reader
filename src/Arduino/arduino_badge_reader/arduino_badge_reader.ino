#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>




#define RST_PIN 9
#define SS_PIN 10

boolean DEBUG = true;

String ssid = "ssid"
String password = "password"

String TARGET_ID = "0";
String TARGET_TYPE = "TCP";
String TARGET_ADDR = "192.168.1.5"; // ip of tcp server
String TARGET_PORT = "11000";

struct ResponseServerTCP {
    char esito[2] = { '0', '\0' };
    char messaggio[30] = { 'R', 'i', 's', 'p', 'o', 's', 't', 'a', ' ', 'n', 'o', 'n', ' ', 'v', 'a', 'l', 'i', 'd', 'a', '\0' };
    char dataOra[25] = { 'n', 'u', 'l', 'l', '\0' };
};


int const BUZZER_PIN = 6;

/*
 * 
 * Communication Protocol:
 *   - $ character transmission start
 *   - & character transmission end
 *   - . separator parameters
 *
 *   Parametri:
 *   - esito: 0,1
 *   - Messaggio: reply message
 *   - dataOra: null || date and time stamping
 */

SoftwareSerial esp8266(7, 8); // RX, TX
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

//prototypes:
String send_AT_ESP8266(String command, int wait, String lastStr); // function used to send AT commands to the wifi module, returns the esp8266 response as a string
String dump_byte_array(byte* buffer, byte bufferSize); // returns the string containing the tag identification code
ResponseServerTCP timbra(String codiceIdentificativo); // Sends the identification code to the server and returns the response
void chiudiConnesione(); // close the connection

void setup()
{

    String response;    
    pinMode(BUZZER_PIN, OUTPUT);
    
    delay(5000);

    //RFID
    SPI.begin();
    mfrc522.PCD_Init();

    if (DEBUG)
        Serial.begin(9600);

    esp8266.begin(9600);
      lcd.begin(16, 2);


    if (DEBUG)
        Serial.println("Inizializzazione modulo wifi..."); // init wifi module
    delay(100);
    response = send_AT_ESP8266("AT+RST", 7000, "ready"); // reset esp8266

    if (response == "TIMEOUT")
     if (DEBUG)
        Serial.println("Errore: TIMEOUT!");
        
    else
     if (DEBUG)
        Serial.println("- Completato"); // completed

    // while(true);

    if (DEBUG)
        Serial.println("Impostazione connesioni multiple...");
    delay(100);
    response = send_AT_ESP8266("AT+CIPMUX=1", 5000, "OK"); // set multiple connections

    if (response == "TIMEOUT") {
if (DEBUG)
        Serial.println("Errore: TIMEOUT!");
        lcd.print("TIMEOUT");

        while (true)
            ;
    }
    else {
      if (DEBUG)
        Serial.println("- Completato");
    }

    String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\""; // I compose the string for wifi

  lcd.clear();
  lcd.print("Connesione WIFI");
  lcd.setCursor(0,1);
  lcd.print(ssid);

if (DEBUG)
    Serial.println("Connessione alla rete " + ssid + " in corso...");
    delay(100);
    response = send_AT_ESP8266(cmd, 20000, "OK");  // connect to wifi

    if (response == "TIMEOUT") {

if (DEBUG)
        Serial.println("Errore: TIMEOUT!");

          lcd.clear();
          lcd.print("WIFI TIMEOUT!");

        while (true);
    }
   


 digitalWrite(BUZZER_PIN, HIGH);  
 delay(300);                     
 digitalWrite(BUZZER_PIN, LOW);   
      
      if (DEBUG)
        Serial.println("- Completato");
    

    if (DEBUG) {
        Serial.println("Inizializzazione completata"); // initialization completed
        Serial.println("In attesa del TAG...");
    }


    lcd.clear();
    lcd.print("Connessione");
    lcd.setCursor(0,1);
    lcd.print("riuscita!");

    delay(1000);

    lcd.clear();
    lcd.print("In attesa TAG...");
    
}

void loop()
{

    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

 digitalWrite(BUZZER_PIN, HIGH);  
 delay(300);                     
 digitalWrite(BUZZER_PIN, LOW);   

   lcd.clear();
   lcd.print("Lettura");
   lcd.setCursor(0,1);
   lcd.print("in corso...");
      
        
        String codiceIdentificativo = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size); // identification code
        if (DEBUG)
        {
        Serial.print(F("Card UID:"));
        Serial.println(codiceIdentificativo);
        }

        ResponseServerTCP response = timbra(codiceIdentificativo);



if(DEBUG){
        Serial.print("Esito: ");
        Serial.println(response.esito);
        Serial.print("Messaggio: ");
        Serial.println(response.messaggio);
        Serial.print("Data e ora: ");
        Serial.println(response.dataOra);
}

if(response.esito[0] == '0')
{
  digitalWrite(BUZZER_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(BUZZER_PIN, LOW);    // turn the LED off by making the voltage LOW



 scrollingLineLeft(String(response.messaggio)," ",9000);
 }
else
{
  digitalWrite(BUZZER_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);                       // wait for a second
  digitalWrite(BUZZER_PIN, LOW);    // turn the LED off by making the voltage LOW
  delay(100); 
  digitalWrite(BUZZER_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(600);                       // wait for a second
  digitalWrite(BUZZER_PIN, LOW);


 scrollingLineLeft(String(response.messaggio),String(response.dataOra),11000);
  
}
   chiudiConnesione();
   



     if (DEBUG) {
      
        Serial.println("In attesa del TAG...");
    } 
    
   lcd.clear();
   lcd.print("In attesa TAG...");}
}

String send_AT_ESP8266(String command, int wait, String lastStr)
{

    int findLastStr = -1;
    unsigned long timeout;
    String responseSTR = "";

    esp8266.println(command);

    delay(20);

    timeout = millis() + wait;

    while (timeout > millis() && findLastStr < 0) {
        if (esp8266.available()) {
            char c = esp8266.read();

            responseSTR += c;

            findLastStr = responseSTR.indexOf(lastStr);
        }
    }

    if (findLastStr < 0) {
        responseSTR = "TIMEOUT";
        //  Serial.println("------------------------------------------------------");
    }

    return responseSTR;
}

String dump_byte_array(byte* buffer, byte bufferSize)
{

    String tag = ""; // = String(*buffer);

    for (byte i = 0; i < bufferSize; i++) {
        tag += String(buffer[i]);
    }
    return tag;
}

ResponseServerTCP timbra(String codiceIdentificativo)
{
    ResponseServerTCP responseTimbratura;

    //compongo la stringa per la connessione al server
    String cmd = "AT+CIPSTART=" + TARGET_ID;
    cmd += ",\"" + TARGET_TYPE + "\",\"" + TARGET_ADDR + "\"";
    cmd += "," + TARGET_PORT;

    if (DEBUG)
        Serial.println("Stabilisco la connesione con il server: " + TARGET_ADDR);
    delay(100);
    String response = send_AT_ESP8266(cmd, 7000, "OK"); // stabilisco connessione col server


        if (response == "TIMEOUT") {
         if (DEBUG) 
            Serial.println("Errore: TIMEOUT!");

            responseTimbratura.messaggio[0] = 'S';
            responseTimbratura.messaggio[1] = 'E';
            responseTimbratura.messaggio[2] = 'R';
            responseTimbratura.messaggio[3] = 'V';
            responseTimbratura.messaggio[4] = 'E';
            responseTimbratura.messaggio[5] = 'R';
            responseTimbratura.messaggio[6] = ' ';
            responseTimbratura.messaggio[7] = 'N';
            responseTimbratura.messaggio[8] = 'O';
            responseTimbratura.messaggio[9] = 'N';
            responseTimbratura.messaggio[10] = ' ';
            responseTimbratura.messaggio[11] = 'D';
            responseTimbratura.messaggio[12] = 'I';
            responseTimbratura.messaggio[13] = 'S';
            responseTimbratura.messaggio[14] = 'P';
            responseTimbratura.messaggio[15] = 'O';
            responseTimbratura.messaggio[16] = 'N';
            responseTimbratura.messaggio[17] = 'I';
            responseTimbratura.messaggio[18] = 'B';
            responseTimbratura.messaggio[19] = 'I';
            responseTimbratura.messaggio[20] = 'L';
            responseTimbratura.messaggio[21] = 'E';
            responseTimbratura.messaggio[22] = '\0';

            return responseTimbratura;
        }
        else
 if (DEBUG) 
            Serial.println("- Connesione stabilita");
    

    String request = codiceIdentificativo + "\r\n";

    cmd = "AT+CIPSEND=";
    cmd += TARGET_ID + "," + request.length(); // calculating the length of bytes to be sent

    if (DEBUG)
        Serial.println("Invio il numero di byte della richiesta...");
    delay(100);
    response = send_AT_ESP8266(cmd, 5000, ">"); 
    if (DEBUG) {

        if (response == "TIMEOUT")
            Serial.println("Errore: TIMEOUT!");

        else
            Serial.println("- Completato");
    }

    // while(true);

    if (DEBUG)
        Serial.println("Invio il codice identificativo...");
    delay(100);

    response = send_AT_ESP8266(request, 10000, "&"); // sending the identification code
   
        if (response == "TIMEOUT") {
           if (DEBUG) 
            Serial.println("Errore: TIMEOUT!");

            responseTimbratura.messaggio[0] = 'T';
            responseTimbratura.messaggio[1] = 'I';
            responseTimbratura.messaggio[2] = 'M';
            responseTimbratura.messaggio[3] = 'E';
            responseTimbratura.messaggio[4] = 'O';
            responseTimbratura.messaggio[5] = 'U';
            responseTimbratura.messaggio[6] = 'T';
            responseTimbratura.messaggio[7] = '\0';

            return responseTimbratura;
 }

        else
        if (DEBUG) 
            Serial.println("- Completato");
    

    // while(true);

    int indexInizioTrasmissione = response.indexOf("$");

    int indexFineTrasmissione = response.indexOf("&");

    if ((indexInizioTrasmissione == -1) || (indexFineTrasmissione == -1)) {
        if (DEBUG)
            Serial.println("Errore: stringa non riconosciuta"); // Error: unrecognized string

        return responseTimbratura;
    }

    response = response.substring(indexInizioTrasmissione + 1, indexFineTrasmissione);
    if (DEBUG) {
        Serial.print("Stringa da parsare: ");
        Serial.println(response);
    }
    delay(100);

    int i = 0;
    int parametro = 0;
    int j = 0;

    while (i < response.length()) { // parse response
        if (response[i] == '.') {
            parametro++;
            j = -1;
           
        }
        else {
            if (parametro == 0) {
               
                responseTimbratura.esito[j] = response[i];
                responseTimbratura.esito[j + 1] = '\0';
            }

            else if (parametro == 1) {
              
                responseTimbratura.messaggio[j] = response[i];
                responseTimbratura.messaggio[j + 1] = '\0';
            }
            else if (parametro == 2) {
               
                responseTimbratura.dataOra[j] = response[i];
                responseTimbratura.dataOra[j + 1] = '\0';
            }
        }
        j++;
        i++;
    }
   
    response = "";

    return responseTimbratura;
}

void chiudiConnesione()
{
    String response = "";

    if (DEBUG)
        Serial.println("Chiusura connessione...");

    delay(100);
    response = send_AT_ESP8266("AT+CIPCLOSE=0", 5000, "OK"); // close the connection

    if (DEBUG) {
        if (response == "TIMEOUT")
            Serial.println("Errore: TIMEOUT!");
        else

            Serial.println("-Connesione chiusa");
    }
}


int arrayCharLength(char * arrayChar)
{

  
int i = 0;


 while(arrayChar[i] != '\0')
  i++;
 i++;


 return i++;
  
  
  
}


void scrollingLineLeft(String scrollLine,String fixLine,int wait) // scroll first row and fix second row
{
  Serial.println(scrollLine);
int scrollCursor = 15;
int stringStart= 0;
int stringStop = 0;
unsigned long timeout = millis() + wait;
lcd.clear();
//delay(1000);
while(millis() <timeout)
{
lcd.setCursor(scrollCursor, 0);
  lcd.print(scrollLine.substring(stringStart,stringStop));
  lcd.setCursor(0, 1);
  lcd.print(fixLine);
  delay(350);
  lcd.clear();

  if(stringStart == 0 && scrollCursor > 0){
    
    scrollCursor--;
    stringStop++;
  } else if (stringStart == stringStop){
    
    stringStart = stringStop = 0;
    scrollCursor = 15;
  } else if (stringStop == scrollLine.length() && scrollCursor == 0) {
   
    stringStart++;
  } else {
   
    stringStart++;
    stringStop++;
  }

 
}  

 }

