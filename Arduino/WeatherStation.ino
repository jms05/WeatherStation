
class WeatherData{
   public: float temp;
   public: float humi;
   public: float pressu;
   public: int rain;
   public: float light;

  public: String toString(){
    String ret = "";
  //  ret+=data.temp+;
    return ret;
  }
};

//Global vars
float tempR;
float humiR;
WeatherData data; 



void config_Transmiter(void){
  
}
void config_RTC(void){
  
}

void desliga_Transmiter(void){
  
}

void liga_Transmiter(void){
  
}

void config_Tem_and_HumSens(void){
  
}

void config_PressureSens(void){
  
}

void config_LightSens(void){
  
}

void config_RainSens(void){
  
}

void config_Sensors(void){
  config_Tem_and_HumSens();
  config_PressureSens();
  config_LightSens();
  config_RainSens();
  
}

void config_Control(void){
  config_Transmiter();
  config_RTC();
}

void initializeVars(void){
  
}
void setup() {
  // put your setup code here, to run once:
  initializeVars();
  config_Control();
  desliga_Transmiter();
  config_Sensors();

}

float readPress(void){
  return 0;
}

int readRain(void){
  return 0;
}

float readLight(void){
 return 0;
}

void readTempHum(void){
  //escrever nas globais
}

void readDataSensors(void){
  //vai ler para var global
  readTempHum();
  data.temp = tempR;
  data.humi = humiR;
  data.pressu=readPress();
  data.rain=readRain();
  data.light=readLight();
  //desligar os sensores em cada metodo

}

void sendData(void){
  liga_Transmiter();
  String dados = data.toString();
  desliga_Transmiter();
}


void desligaS(int segundos){
  //config interrup
  // sleep modo
  
}
void desligaM(int minutos){
  desligaS(minutos*60);
}

void desligaH(int horas, int minutos){
  desligaM(horas*60+minutos);
}
void loop() {
  // put your main code here, to run repeatedly:
  readDataSensors();
  sendData();
  desligaM(15);
    
}
