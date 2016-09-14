

#include "DHT.h"
#include <Wire.h>
#include <DS3231.h>              
#include <avr/sleep.h>   
#include <SPI.h>
#include <RF24.h>

#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define PHOTOPIN A1
#define RAINPIN A0
#define UVPIN A2
#define REF3_3 A3
#define SLEEPCICLE 30 // time to sleep Value * 30 Sec
#define TRIES 3    // number of tries to sent one record
#define ERRO_VALUE -1

#define MAIN_SWITCH_PIN 8 // transistor connected to this pint to act as seitch to torn off/on all Sensors
//the RTC 
#define TIME_MESURE 1 
#define WAKE_PIN 2
#define MINUTES_TO_SLEEP 15



//for nrf24 debug
int serial_putc( char c, FILE * ) 
{
  Serial.write( c );
  return c;
} 

//for nrf24 debug
void printf_begin(void)
{
  fdevopen( &serial_putc, 0 );
}

//volatile DS3231 clock;
//volatile RTCDateTime dt;
 DS3231 clock;
 RTCDateTime dt;

DHT dht(DHTPIN, DHTTYPE);

//Radio comunication
RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL,0xF0F0F0F0D2LL };

//where the data from sensors are save
float light;
int rain;
float humi;
float temperature;
float pressure;
float uvlevel;
//PDU
char dataSend[40];


void wakeUpFunction() {  
  // execute code here after wake-up before returning to the loop() function  
  // timers and code using timers (serial.print and more...) will not work here.  
  // we don't really need to execute any special functions here, since we  
  // just want the thing to wake up  
  noInterrupts();
  Serial.println("wakeUpFunction:Woke up");
  digitalWrite(MAIN_SWITCH_PIN,HIGH);
  delay(5000); //5sec to warm up the sensors
  my_main_Func(); //do everything from weatherStation
  //sleepNow();
  //delay(100);
  Serial.println("wakeUpFunction:Exit");
} 

void wakeUpFunction2() {  
  // execute code here after wake-up before returning to the loop() function  
  // timers and code using timers (serial.print and more...) will not work here.  
  // we don't really need to execute any special functions here, since we  
  // just want the thing to wake up  

  Serial.println("wakeUpFunction2");
  //digitalWrite(MAIN_SWITCH_PIN,HIGH);
  //delay(5000); //5sec to warm up the sensors
  //my_main_Func(); //do everything from weatherStation
  //sleepNow();
  delay(1000);
  //Serial.println("wakeUpFunction:Exit");
} 

 void sleepNow() { 
    Serial.println("sleepNow:Go Sleep");
    setAlarmNextTime(0);  //the alrm is set to the next 15 min
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // sleep mode is set here  
    sleep_enable();                         // enables the sleep bit in the mcucr register  
    attachInterrupt(digitalPinToInterrupt(WAKE_PIN), wakeUpFunction2, CHANGE);
    digitalWrite(MAIN_SWITCH_PIN,LOW); //turn sensors off   
    sleep_mode();                           // here the device is actually put to sleep!!  
    // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP  
    //call the wake funciton

    //exit wake
    Serial.println("sleepNow after exit wake");
    sleep_disable();         // first thing after waking from sleep: disable sleep...  
    detachInterrupt(digitalPinToInterrupt(WAKE_PIN));      // disables interrupt 0 on pin 2 so the wakeUpNow code will not be executed during normal running time.  
    interrupts();
    sleepNow();
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


//SETUP----------------------------------------------------------

void setupRF24(){
  radio.begin();
  radio.setChannel(0x4c);
  radio.setAutoAck(1);
  radio.enableDynamicAck();
  radio.setRetries(15,15);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.setPayloadSize(32);
  
  radio.openReadingPipe(1,pipes[0]);
  radio.openWritingPipe(pipes[1]);
  radio.printDetails(); //for Debugging
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


void setAlarmNextTime(bool firstTime){

  if(!firstTime){
    int minutes = clock.getDateTime().minute;
    int nextminuteAlarm=(minutes+MINUTES_TO_SLEEP) %60;
    //Serial.println(nextminuteAlarm);
    clock.armAlarm1(false);
    clock.clearAlarm1();
    clock.setAlarm1(0, 0, nextminuteAlarm, 1, DS3231_MATCH_M_S);
    // MINUTES_TO_SLEEP
    RTCAlarmTime a1;
    if (clock.isArmed1()){
      Serial.println("setAlarmNextTime:Ligado");
      a1 = clock.getAlarm1();
      Serial.println("setAlarmNextTime:Next alarm: ");
      String data = String("setAlarmNextTime:")+String(clock.dateFormat("d-m-Y H:i:s", a1));
      Serial.println(data);
    }
   }else{
     //firts time
      int second = clock.getDateTime().second;
      int nextsecondAlarm=(second+10) %60;
      //Serial.println(nextminuteAlarm);
      clock.armAlarm1(false);
      clock.clearAlarm1();
      clock.setAlarm1(0, 0, 0, nextsecondAlarm, DS3231_MATCH_S);
      // MINUTES_TO_SLEEP
      RTCAlarmTime a1;
      if (clock.isArmed1()){
        Serial.println("setAlarmNextTime:Ligado");
        a1 = clock.getAlarm1();
        Serial.println("setAlarmNextTime:Next alarm: ");
        String data = String("setAlarmNextTime:")+String(clock.dateFormat("d-m-Y H:i:s", a1));
        Serial.println(data);
      }
  }
}


void setupRTC(){
  clock.begin();
  clock.enableOutput(false);
  //setAlarmNextTime(1);

}


void setup() {
  
  Serial.begin(9600);

  pinMode(MAIN_SWITCH_PIN,OUTPUT);
  digitalWrite(MAIN_SWITCH_PIN,HIGH);
  delay(10000);
  //sensors
  pinMode(PHOTOPIN, INPUT);
  pinMode(RAINPIN, INPUT);
  pinMode(UVPIN,INPUT);
  pinMode(REF3_3,INPUT);
 

  //Sensor DHT22
  dht.begin();
  //radio
  setupRF24();
  //vars
  setup_vars();

  //RTC
  setupRTC();
  
  // set alarm pin as input and Interrupt

  pinMode(WAKE_PIN, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(WAKE_PIN), wakeUpFunction2, CHANGE);
  delay(2000);
 // wakeUpFunction();
  
}

//END SETUP-----------------------------------------


//WORK WITH PDU ------------------------
void sendData(){
  bool ok=1;
  radio.powerUp();
  delay(10);
  ok=radio.write(&dataSend,strlen(dataSend));
  delay(10);
  radio.powerDown();
}



void buildStringSend(){

  String data = String(String(temperature,2)+";"+String(humi,2)+";"+light+";"
  +String(pressure,2)+";"+rain+";"
  +String(uvlevel,2));
  
  data.toCharArray(dataSend,40);
}


//END WORK WITH PDU ------------------------

//TAKE MESURES ------------------------
float readLight(){
  int AR=averageAnalogRead(PHOTOPIN); //0 muita luz 1023 escuro
  return map(AR,1023,10,0,100);
  /*Serial.print("AR: ");
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
  lightLevel = (500.0 / RLDR);*/

}

float readUV(){
  //https://learn.sparkfun.com/tutorials/ml8511-uv-sensor-hookup-guide
  int uvLevel = averageAnalogRead(UVPIN);
  int refLevel = averageAnalogRead(REF3_3);
  float outputVoltage = 3.3 / refLevel * uvLevel;
  float uvlevel = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level
  return uvlevel;
}
 
float readRain(){
  int AR=averageAnalogRead(RAINPIN); //0 muita chuva 1023 seco
  return map(AR,1023,0,0,100);
  }

//END MESURES ------------------------


void my_main_Func(){ //this is the main where everything is controled
  Serial.println("my_main_Func:Taking mesures");
    humi = dht.readHumidity();
    temperature = dht.readTemperature();
    light= readLight();
    rain=readRain();
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
    Serial.print("my_main_Func:SEND: ");
    Serial.println(dataSend);
    for(int c=0;c<TRIES;c++){
        Serial.print("Time: ");
        Serial.println(c);
        sendData();
        delay(2000); // each try with 2 sec delay
    
    }
}
bool firstTime =1;

void loop() {
  dt = clock.getDateTime();

  Serial.println(clock.dateFormat("d-m-Y H:i:s - l", dt));
  if(firstTime){
    Serial.println("loop:Entering sleep"); 
    firstTime=0;
    sleepNow();
  }
   Serial.println("loop:Out"); 
  delay(20000);
  //delay(1000); // sleep function called here
 // sleepNow(); 
  //delay(1000);
  /*if(firstTime){
    Serial.println("1º time");
    my_main_Func(); //take the firsr mesure
    firstTime=0;
  }
  
  if (clock.isAlarm1())
  {
    Serial.println("loop:Entering sleep"); 
    delay(100); // sleep function called here
    sleepNow(); 
  }else{
    Serial.println("loop:not sleep"); 
  }
  delay(50000);*/

}


