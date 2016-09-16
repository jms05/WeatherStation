#include "DHT.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define PHOTOPIN A1
#define RAINPIN A0
#define UVPIN A2
#define REF3_3 A3
//#define SLEEPCICLE 30 // time to sleep Value * 30 Sec
#define MIN_SLEEP 15  //minutes to sleep between mesures
#define TRIES 2    // number of tries to sent one record
#define ERRO_VALUE -1

#define MAIN_SWITCH_PIN 8

#define MY_RF_CH 0x53

DHT dht(DHTPIN, DHTTYPE);
RF24 radio(9,10);

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL,0xF0F0F0F0D2LL }; //NRF communication addrs

//where the data from sensors are save
float light;
int rain;
float humi;
float temperature;
float pressure;
float uvlevel;
//PDU
char dataSend[40];

//compute the millis to sleep
long sleepTimeAux = MIN_SLEEP*60; //secons to sleep
long sleepTime =sleepTimeAux*1000; //millis to sleep

void setupRF24(){
  radio.begin();
  radio.setChannel(MY_RF_CH);
  radio.setAutoAck(1);
  radio.enableDynamicAck();
  radio.setRetries(15,15);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.setPayloadSize(32);
  
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  //radio.printDetails(); //for Debugging
  //radio.powerDown();
  delay(5);
}

void setup_vars(){
  light=ERRO_VALUE;
  rain=ERRO_VALUE;
  humi=ERRO_VALUE;
  temperature=ERRO_VALUE;
  pressure=ERRO_VALUE;
  uvlevel=ERRO_VALUE;
  dataSend[0]='\0';

}

void setup() {
  //Serial.begin(9600);
  pinMode(MAIN_SWITCH_PIN,OUTPUT); //transistor to poweroff the sensors when idle mode
  //digitalWrite(MAIN_SWITCH_PIN,LOW);
  //sensors
  pinMode(PHOTOPIN, INPUT);
  pinMode(RAINPIN, INPUT);
  dht.begin();
  //radio
  setupRF24();
  //vars
  setup_vars();
 // Serial.println("End Setup");
}


void sendData(){
  radio.powerUp();
  delay(10);
  radio.write(&dataSend,strlen(dataSend));
  delay(10);
  radio.powerDown();
}


float readAnalogMAP(int pin){
  int AR=averageAnalogRead(pin); //0 muita luz 1023 escuro
  int pc= map(AR,1012,10,0,100);
  if(pc<=0) return 0;
  if(pc>=100) return 100;
  return pc;
}

float readUV(){
  //https://learn.sparkfun.com/tutorials/ml8511-uv-sensor-hookup-guide
  int uvLevel = averageAnalogRead(UVPIN);
  int refLevel = averageAnalogRead(REF3_3);
  float outputVoltage = 3.3 / refLevel * uvLevel;
  float uvlevel = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level
  return uvlevel;
}
/*
float readRain(){
  int AR=averageAnalogRead(RAINPIN); //0 muita chuva 1023 seco
  return map(AR,1012,0,10,100);
  }


float readLight(){
  int AR=averageAnalogRead(PHOTOPIN); //0 muita luz 1023 escuro
  return map(AR,1012,0,10,100);
  Serial.print("AR: ");
  Serial.println(AR);
  
  Serial.print("Lux: ");
  Serial.println(lightLevel);*/
  
  /*float Vread = AR * 0.0048875855327468; //5/1023
  Serial.print("VRead: ");
  Serial.println(Vread);
  if(Vread==5.0){
    Vread=4.9999;
  }
  float RLDR = (-10.0*Vread)/(Vread-5.0);     
   // R1 = 10,000 Ohms , Vin = 5.0 Vdc.

  Serial.print("R_Sensor: ");
  Serial.println(RLDR);
  if(RLDR==0.0){
    RLDR=.001;
  }
  lightLevel = (500.0 / RLDR);

}
*/
void buildStringSend(){

  String data = String(String(temperature,2)+";"+String(humi,2)+";"+light+";"
  +String(pressure,2)+";"+rain+";"
  +String(uvlevel,2));
  
  data.toCharArray(dataSend,40);
 // Serial.println(dataSend);
}

//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++){
     runningValue += analogRead(pinToRead);
     delay(50);
  }
  runningValue /= numberOfReadings;
  return(runningValue);  
}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void readData(){
  humi = dht.readHumidity();
  temperature = dht.readTemperature();
  light=readAnalogMAP(PHOTOPIN);
  rain=readAnalogMAP(RAINPIN);
}
void loop() {
  digitalWrite(MAIN_SWITCH_PIN,HIGH); //turn on the sensors
  delay(1000); //wait for snesors to warm up
  readData(); //read data from sensors
  digitalWrite(MAIN_SWITCH_PIN,LOW); //turn off the sensors to power save
  
  // the next block will check if any values read is not ok if so will "mark" it with the error value -1
  if(isnan(humi)){
    humi=ERRO_VALUE;
  }
  if(isnan(temperature)){
    temperature=ERRO_VALUE;
  }
  if(isnan(light)){
    light=ERRO_VALUE;
  }
  if(isnan(rain)){
    rain=-ERRO_VALUE;
  }
  if(isnan(uvlevel)){
    uvlevel=ERRO_VALUE;
  }
  buildStringSend();
  
  //this cicle will send the data TRIES times with a interval of 2 seconds
  for(int c=0;c<TRIES;c++){
    //Serial.print("Time: ");
    //Serial.println(c);
    sendData();
    delay(2000); // each try with 2 sec delay
  }
  delay(sleepTime);
}
