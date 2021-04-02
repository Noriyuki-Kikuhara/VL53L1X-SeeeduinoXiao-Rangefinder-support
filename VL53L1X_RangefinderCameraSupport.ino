#include <Wire.h>
#include <VL53L1X.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
#define RST_PIN -1
SSD1306AsciiWire oled;
VL53L1X sensor;

int pin = 1;     // Buzzer pin
#define selectPin 6                 // Button for Select
int preset;
int offsetpreset;
bool setflange;
int flange;
int channel;
bool setch;
int Cm;
int feet;
int savr;
int sens;
int sens_dump;

const int presetLength = 10;
int presetList[presetLength] =      //BEEP preset Centimeter
{
153,  //APOQUALIA28 1.4m position
150,  //SUMMILUX50mm 1.5m position
120,  //SUMMILUX50mm 1.2m position
118,  //APOQUALIA28 1m position
100,  //SUMMILUX50mm 1m position
98,  //SUMMICRON 5cm LTM Shortest shooting distance
95,  //SUMMILUX50mm Shortest shooting distance
92,  //APOQUALIA28の0.8m position
65,  //APOQUALIA28の0.6m position
50,  //SOOKY-LMT Shortest shooting distance ??(not test)
};

#define offsetPin 8   //測距センサーとフィルム面の差+調整用 Laser sensor and film offset amount
const int  cameraFlange = 4;
int camera_offsetList[cameraFlange] =
{
  0,
24, // M9
29, // DIII
31, // M2 IIIg
};


void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }
 ////Long 暗所Max360cm/強い外光下73cm Midium 暗所Max290cm/強い外光下76cm Short 暗所Max136cm/外光下135cm
 ////See data sheet 2.5.1　＞＞ https://akizukidenshi.com/download/ds/st/vl53l1x.pdf
  sensor.setDistanceMode(VL53L1X::Short);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);
#if RST_PIN >= 0
  oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
#endif // RST_PIN >= 0
  oled.setFont(Adafruit5x7);
  oled.set2X();
  pinMode(selectPin, INPUT_PULLUP);
  pinMode(7, OUTPUT);
  pinMode(offsetPin,INPUT_PULLUP);
  
/*
///// ミ・ミ・ミッ・ド・ミッ・・ソ /////
  tone(pin,659,150);
  delay(100);
  tone(pin,659,150);
  delay(200);
  tone(pin,659,150);
  delay(200);
  tone(pin,523,250);
  delay(100);
  tone(pin,659,150);
  delay(250);
  tone(pin,784,150);
  delay(520);
  tone(pin,392,150);
  delay(500); 
////////////////////////////////////
*/
}


void loop(){
  BeepDistance();
  setch = digitalRead(selectPin);
    if(setch == LOW){
    setBeepDistance();
    delay(100);
    }else{
  setflange = digitalRead(offsetPin);
    if(setflange == LOW){
    setCameraFlange();
    }else{
    oled_Disp();
    }
}
}

////////////////////////////////////

 int ReadSens_Avr(){
  setch = digitalRead(selectPin);
  if(setch == LOW){
    delay(200);
  }else{
  setflange = digitalRead(offsetPin);
  if(setflange ==  LOW){
    delay(200); 
  }else{
  int i;
  int savr;
  int sens_dmp;
  sens_dmp = 4 ;                       //センサ値の蓄積回数　measurement data accumulations
  for ( i = 0; i < sens_dmp; i++){
    savr = savr += sensor.read();  // VL53L1Xセンサから読み取り sensor read
  }
  savr = (savr / sens_dmp ); // 上のfor文の蓄積回数で割って平均出すDivide the number of accumulations
 
 }}}
 
void oled_Disp(){
  float sens;
  sens = (ReadSens_Avr() + flange);  //フランジバック修正 Laser sensor and film offset amount
  float Cm;
  Cm = sens /10;
  float feet;                     //昔の北米輸出用のレンズはfeet表記なのでfeet表示あった方が良さげ
  feet = sens / 304.8 ;           //ミリからfeetへ単位変換  Extra feet display
  oled.clear();
  oled.print(Cm , 1 );
  oled.println(" cm");
  oled.print(feet , 1 );
  oled.println(" feet");
}

 bool buttonPressd(){
  return !digitalRead(selectPin);
}

 bool offsetButtonPressd(){
  return !digitalRead(offsetPin);
 }

 void setBeepDistance(){
  if(buttonPressd()){
  getdistanceTable();
  }
}

void getdistanceTable(){
  if(buttonPressd()){
       ++preset;
      if(preset >= presetLength)
        preset = 0;
      channel = presetList[preset];    
  oled.clear();
  oled.println("Set Distance");
  oled.print(channel);
  oled.print(" cm");
  delay(1000);
  }
}

void setCameraFlange(){
  if(offsetButtonPressd()){
    getoffsetTable();
  }
}

void getoffsetTable(){
  if(offsetButtonPressd()){
    ++offsetpreset;
    if(offsetpreset >= cameraFlange)
    offsetpreset = 0;
    flange = camera_offsetList[offsetpreset];
    oled.clear();
    oled.println("set offset");
    oled.print(flange);
    delay(1000);
  }
}

void beep(){

/////　シ・ミ〜 Mari.. coin sound /////
     tone(pin,1976,220) ;
     delay(80) ;
     tone(pin,2636,350) ;
     delay(250) ;

}

void BeepDistance(){
  channel = presetList[preset];  
  int sens;
  sens = (ReadSens_Avr() + flange);
  int Cm;
  Cm = sens /10;
  if(Cm == channel){
    beep();
  }
  return;
}
