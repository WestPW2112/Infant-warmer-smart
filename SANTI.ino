#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>
#include <TimerOne.h>

//////DIMMER//////////
volatile int i=0;             
volatile boolean zero_cross=0; 
int AC_pin = 3;                
int dim = 128;                  
int freqStep = 75;

/////sensor heart//////
#define heartSensor A0
int valDetakJantung;

//////sensor DS//////
#define ONE_WIRE_BUS 9
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
float valSuhuBayi;

//////sensor DHT//////
#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float valTemperature;
float valHumidity;

/////////LCD////////
LiquidCrystal_I2C lcd(0x27, 20, 4);

/////////PUSH BUTTON///////
int Next = 0;
int Ok = 0;
int Back = 0;

/////PIN/////
#define KIPAS 10
int valkipas = 0;
int vallampu = 0;

bool split = false;
String sdata, data[10];

void setup() {
  Serial.begin(9600);
  sensorSuhu.begin();
  dht.begin();
  lcd.begin();

  pinMode(AC_pin, OUTPUT);
  attachInterrupt(0, zero_cross_detect, RISING);
  Timer1.initialize(freqStep);                     
  Timer1.attachInterrupt(dim_check, freqStep); 

  pinMode(KIPAS,OUTPUT);
  pinMode(4,INPUT_PULLUP);
  pinMode(5,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);
  pinMode(7,OUTPUT);
}

void zero_cross_detect() {   
  zero_cross = true; 
  i=0;
  digitalWrite(AC_pin, LOW);     
}

void dim_check() {                 
  if(zero_cross == true) {             
    if(i>=dim) {                   
      digitalWrite(AC_pin, HIGH); // lampu menyala     
      i=0;  // reset waktu step counter                       
      zero_cross = false; //reset zero cross detection
    }
    else {
      i++; // increment time step counter                   
    }                               
  }                                 
}

void loop() {
  
  menu();

}

void sender(){
    
    
    int myBPM = analogRead(heartSensor);
    
    int detak1 = myBPM; delay(300);
    int detak2 = myBPM; delay(300);
    int detak3 = myBPM; delay(300);
    int detak4 = myBPM; delay(300);
    int detak5 = myBPM; delay(300);
    valDetakJantung = (detak1 + detak2 + detak3 + detak4 + detak5)/35;

    valTemperature = dht.readTemperature()+2;
    int valtemp = valTemperature*100; 

    valSuhuBayi = ambilSuhu()+3;
    int valsuhu = valSuhuBayi*100; 
    
    String kirim = "";
    kirim += "#";
    kirim += valDetakJantung;
    kirim += "#";
    kirim += valtemp;
    kirim += "#";
    kirim += valsuhu;
    kirim += "#";
    kirim += valkipas;
    kirim += "#";
    kirim += dim;
    kirim += "#";
    kirim += "$";

    Serial.println(kirim);
}

void spliter(){
  while (Serial.available()){
    char inchar = Serial.read();
    sdata += inchar;
    if (inchar == '$'){
      split = true;
    }
    if (split){
      int q = 0;
      for (int i = 0; i < sdata.length(); i++){
        if (sdata[i] == '#'){
          q++;
          data[q] = "";
        }
        else{
          data[q] += sdata[i];
        }
      }
      
      //valDetakJantung = data[1].toInt();
      //valTemperature = data[2].toInt();
      //valSuhuBayi = data[3].toInt();
      valkipas = data[4].toInt();
      if (data[5].toInt() > 128){otomatis();}
      if (data[5].toInt() <= 128){dim = data[5].toInt();}

      if(valkipas == 1){digitalWrite(10,LOW);}
      if(valkipas == 0){digitalWrite(10,HIGH);}
      if(valkipas == 2){if(valTemperature >= 36){digitalWrite(10,LOW);} else{digitalWrite(10,HIGH);}}
      
      split = false;
      sdata = "";
    }
  }
}

void notif(){
  if (valSuhuBayi >= 36.4){
    digitalWrite(7,HIGH); delay(500);
    digitalWrite(7,LOW); delay(500);
    digitalWrite(7,HIGH); delay(500);
    digitalWrite(7,LOW); delay(500);
    digitalWrite(7,HIGH); delay(500);
    digitalWrite(7,LOW); delay(500);
  }

  if(valTemperature <= 35 || valTemperature >= 37 || valDetakJantung <= 70 || valDetakJantung >= 110){
    digitalWrite(7,HIGH); delay(500);
    digitalWrite(7,LOW); delay(500);
    digitalWrite(7,HIGH); delay(500);
    digitalWrite(7,LOW); delay(500);
  }
}

void menu(){

  Home:
  while(1){

    //otomatis();
    sender();
    spliter();
    notif();
    if(valkipas == 2){if(valTemperature >= 36){digitalWrite(10,LOW);} else{digitalWrite(10,HIGH);}}
    
    lcd.setCursor(0,0); lcd.print("   INFANT WARMER    ");
    lcd.setCursor(0,1); lcd.print("                    ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BAYI    CTRL    ALAT");
   
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto SUHUALAT;}
    if (Ok == HIGH) { delay(200); lcd.clear(); goto KONTROLKIPAS;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto SUHUBAYI;}
  }

  SUHUBAYI:
  while(1){
    
    valSuhuBayi = ambilSuhu()+3;
    sender();
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("     SUHU BAYI      ");
    lcd.setCursor(5,1); lcd.print(valSuhuBayi);   lcd.setCursor(12,1); lcd.print("C");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("DETAK           BACK");
    delay(1000);
    Next = digitalRead(6);
    //Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto Home;}
    //if (Ok == HIGH) { delay(200); lcd.clear(); goto SUHUBAYI0;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto DETAK;}
  }

  DETAK:
  while(1){
    int myBPM = analogRead(heartSensor);
    
    int detak1 = myBPM; delay(300);
    int detak2 = myBPM; delay(300);
    int detak3 = myBPM; delay(300);
    int detak4 = myBPM; delay(300);
    int detak5 = myBPM; delay(300);
    valDetakJantung = (detak1 + detak2 + detak3 + detak4 + detak5)/35;
    sender();
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("     DETAK BAYI     ");
    lcd.setCursor(5,1); lcd.print(valDetakJantung);   lcd.setCursor(12,1); lcd.print("BPM");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("                BACK");
    delay(1000);
    Next = digitalRead(6);
    //Ok = digitalRead(5);
    //Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto SUHUBAYI;}
    //if (Ok == HIGH) { delay(200); lcd.clear(); goto DETAK0;}
  }


  SUHUALAT:
  while(1){
    valTemperature = dht.readTemperature()+2;
    //otomatis();
    sender();
    lcd.clear();
    lcd.setCursor(0,0); lcd.print("     SUHU ALAT      ");
    lcd.setCursor(5,1); lcd.print(valTemperature);   lcd.setCursor(12,1); lcd.print("C");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK            KLMB");
    delay(1000);

    Next = digitalRead(6);
    //Ok = digitalRead(5);
    Back = digitalRead(4);

    //if (Next == HIGH) { delay(200); lcd.clear(); goto KLMBALAT;}
    //if (Ok == HIGH) { delay(200); lcd.clear(); goto SUHUALAT0;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto Home;}
  }

  KONTROLKIPAS:
  while(1){

    lcd.setCursor(0,0); lcd.print("   KONTROL  KIPAS   ");
    lcd.setCursor(5,1); lcd.print("                    ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE    LAMPU");
    
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU;}
    if (Ok == HIGH) { delay(200); lcd.clear();  goto KONTROLKIPAS0;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto Home;}
  }

    KONTROLKIPAS0:
    while(1){
    
    lcd.setCursor(0,0); lcd.print("   KONTROL  KIPAS   ");
    lcd.setCursor(5,1); lcd.print("                    ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK     ON      OFF");
    
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); valkipas = 0; sender(); digitalWrite(10,HIGH); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLKIPAS;}
    if (Ok == HIGH) { delay(200); lcd.clear(); valkipas = 1; sender(); digitalWrite(10,LOW); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLKIPAS;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto Home;}
  }

  KONTROLLAMPU:
  while(1){
   
    lcd.setCursor(0,0); lcd.print("   KONTROL  LAMPU   ");
    lcd.setCursor(0,1); lcd.print("                    ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE         ");
    
   
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Ok == HIGH) { delay(200); lcd.clear();  goto KONTROLLAMPU0;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto Home;}
  }

    KONTROLLAMPU0:
    while(1){
    
    lcd.setCursor(0,0); lcd.print("   KONTROL  LAMPU   ");
    lcd.setCursor(0,1); lcd.print("    KECERAHAN 0%    ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE     NEXT");
  
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU20;}
    if (Ok == HIGH) { delay(200); lcd.clear(); dim = 128; sender(); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLLAMPU;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU;}
  }

  KONTROLLAMPU20:
    while(1){
    
    lcd.setCursor(0,0); lcd.print("   KONTROL  LAMPU   ");
    lcd.setCursor(0,1); lcd.print("    KECERAHAN 20%   ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE     NEXT");
    
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU40;}
    if (Ok == HIGH) { delay(200); lcd.clear(); dim = 100; sender(); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLLAMPU;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU0;}
  }

  KONTROLLAMPU40:
    while(1){
    
    lcd.setCursor(0,0); lcd.print("   KONTROL  LAMPU   ");
    lcd.setCursor(0,1); lcd.print("    KECERAHAN 40%   ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE     NEXT");
    
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU60;}
    if (Ok == HIGH) { delay(200); lcd.clear(); dim = 75; sender(); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLLAMPU;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU20;}
  }

  KONTROLLAMPU60:
    while(1){
    
    lcd.setCursor(0,0); lcd.print("   KONTROL  LAMPU   ");
    lcd.setCursor(0,1); lcd.print("    KECERAHAN 60%   ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE     NEXT");

   
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU80;}
    if (Ok == HIGH) { delay(200); lcd.clear(); dim = 45; sender(); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLLAMPU;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU40;}
  }

  KONTROLLAMPU80:
    while(1){
     
    lcd.setCursor(0,0); lcd.print("   KONTROL  LAMPU   ");
    lcd.setCursor(0,1); lcd.print("    KECERAHAN 80%   ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE     NEXT");
    
    Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Next == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU100;}
    if (Ok == HIGH) { delay(200); lcd.clear(); dim = 25; sender(); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLLAMPU;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU60;}
  }

  KONTROLLAMPU100:
    while(1){
    
    lcd.setCursor(0,0); lcd.print("   KONTROL  LAMPU   ");
    lcd.setCursor(0,1); lcd.print("   KECERAHAN 100%   ");
    lcd.setCursor(0,2); lcd.print("--------------------");
    lcd.setCursor(0,3); lcd.print("BACK    OKE         ");
    
    //Next = digitalRead(6);
    Ok = digitalRead(5);
    Back = digitalRead(4);

    if (Ok == HIGH) { delay(200); lcd.clear(); dim = 0; sender(); lcd.setCursor(0,0); lcd.print("     SELESAI    "); delay(3000); goto KONTROLLAMPU;}
    if (Back == HIGH) { delay(200); lcd.clear(); goto KONTROLLAMPU80;}
  }
}

void otomatis(){
  valTemperature = dht.readTemperature()+2; 
    
    if ( valTemperature <= 35){
      dim = 0; sender();
    }
    if ( valTemperature <= 36 && valTemperature > 35){
      dim = 25; sender();
    }
    if ( valTemperature <= 37 && valTemperature > 36){
      dim = 45; sender();
    }
    if ( valTemperature <= 38 && valTemperature > 37){
      dim = 70; sender();
    }
    if ( valTemperature <= 39 && valTemperature > 38){
      dim = 95; sender();
    }
    if ( valTemperature <= 40 && valTemperature > 39){
      dim = 128; sender();
    }
}

float ambilSuhu()
{
   sensorSuhu.requestTemperatures();
   float suhu = sensorSuhu.getTempCByIndex(0);
   return suhu;   

}