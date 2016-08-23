#include <Narcoleptic.h>

//#include <Sleep_n0m1.h>
#include "DHT.h"
#include <SPI.h>
#include <RF24.h>

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define PHOTOPIN 3
#define RAINPIN A1
#define SLEEPTIME_MIN 0.5


DHT dht(DHTPIN, DHTTYPE);
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL,0xF0F0F0F0D2LL };

//Sleep sleep;
int lightLevel;
int rainLevel;
float humi;
float temperature;
float pressure;
char dataSend[40];

unsigned long sleepTime;

void setupRF24(){
  radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.setRetries(15,15);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.setPayloadSize(32);
  
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  //radio.startListening();
  radio.printDetails(); //for Debugging
  radio.powerDown();
  delay(5);
}

void setup_vars(){
  lightLevel=-999;
  rainLevel=-999;
  humi=-999;
  temperature=-999;
  pressure=-999;
  dataSend[0]='\0';
  sleepTime=60000;

}

void setup() {
  
  Serial.begin(9600); 
  //sensors
  pinMode(PHOTOPIN, INPUT);
  pinMode(RAINPIN, INPUT);
  dht.begin();
  //radio
  setupRF24();
  //vars
  setup_vars();
}


void sendData(){

  radio.powerUp();
 // Serial.print(SendPayload);
  //bool ok = radio.write(&dataSend,strlen(dataSend));
  radio.write(&dataSend,strlen(dataSend));
  /*Serial.print("--SEnd:");
  Serial.println(ok);*/
  radio.powerDown();

}




void buildStringSend(){
  String data = String(String(temperature,2)+";"+String(humi,2)+";"+lightLevel+";"
  +String(pressure,2)+";"+rainLevel);

  data.toCharArray(dataSend,40);
}


void loop() {
  
  humi = dht.readHumidity();
  temperature = dht.readTemperature();
  lightLevel=digitalRead(PHOTOPIN);
  rainLevel = analogRead(RAINPIN);
  if(isnan(humi)){
        humi=-999;
  }
  if(isnan(temperature)){
         temperature=-999;
  }
  if(isnan(lightLevel)){
         lightLevel=-999;
  }
  if(isnan(rainLevel)){
         rainLevel=-999;
  }


  
  buildStringSend();
  
  Serial.println(dataSend);
  for(int c=0;c<3;c++){
    sendData();
    delay(1000);
  }
  
  /*Serial.print("TEMP Max: ");
  Serial.print(temperatureMax);
  Serial.print(" C TEMP Min: ");
  Serial.println(temperatureMin);

  Serial.print("Humi Max: ");
  Serial.print(humiMax);
  Serial.print(" % Humi Min: ");
  Serial.println(humiMin);
  //Serial.println("Enviar");
  //sendData();
  Serial.println("Dormir 5 min");
  //mysleep();
  */
  for(int c=0;c<10;c++){
    delay(30000);
  }
  /*
  delay(30000);
  delay(30000);
  delay(30000);
  delay(30000);
  delay(30000);
  delay(30000);
  delay(30000);
  delay(30000);
  delay(30000);
*/
}
