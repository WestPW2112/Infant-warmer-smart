#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>               
#include <TimeLib.h> 
#include <WiFiClientSecure.h>

//////setup koneksi wifi//////
String SSID = "indopride"; //name wifi
String password = "BAB@2000"; //password wifi
const char* mqtt_server = "broker.hivemq.com";

const long utcOffsetInSeconds = 25200;  // set offset
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org", utcOffsetInSeconds);
int detik, menit, jam, hari, bulan, tahun;

const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client; 
String GAS_ID = "AKfycbxpt8qdzU2NG4BRO0qdLgA8ceSAYZ0zcNjqnXfaPi08rbftCS9n0kbnSJEJxSzGc6Duiw"; 

const char* topic_suhualat  = "/infant/warmer/suhualat/1";
const char* topic_suhubayi  = "/infant/warmer/suhubayi/1";
const char* topic_detak  = "/infant/warmer/detak/1";
const char* topic_kipas  = "/infant/warmer/kipas/1";
const char* topic_lampu  = "/infant/warmer/lampu/1";
const char* topic_in  = "/infant/warmer/in/1";

WiFiClient espClient;
PubSubClient klient(espClient);

unsigned long previousMillis = 0;
const long interval = 5000;
 
/////////////////////////////

bool split = false;
String sdata, data[10];

int valDetakJantung;
float valSuhuBayi;
float valTemperature;
int valdim;
int valkipas;
String namapasien = "null";
  
void setup() {
  Serial.begin(9600);//baudrate
  
  //////setup kipas//////
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  //////////////////////

  //////setup wifi//////
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, password);
  timeClient.begin();

  randomSeed(micros());
  client.setInsecure();
  
  klient.setServer(mqtt_server, 1883);
  klient.setCallback(callback);
  //////////////////////
}

void rtc() {

  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();    // Get Unix epoch time from the NTP server

  detik = second(unix_epoch);
  
}

void database(){

  rtc();

  if (detik == 1){
    if (!client.connect(host, httpsPort)) { return;}

    String url = "/macros/s/" + GAS_ID + "/exec?nama_pasien=" + String(namapasien) + "&suhubayi=" + String(valSuhuBayi) + "&suhualat=" + String(valTemperature) + "&detakjantung=" + String(valDetakJantung);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");

  }
  
}

void kirim(){

  unsigned long currentMillis = millis(); 
 
    if (currentMillis - previousMillis >= interval) { 
    previousMillis = currentMillis;
      
      String msgStrSB = "";
      msgStrSB = String(valSuhuBayi);
      byte arrSizeSB = msgStrSB.length() + 1;
      char msgSB[arrSizeSB];
      msgStrSB.toCharArray(msgSB, arrSizeSB);
      klient.publish(topic_suhubayi, msgSB);
      msgStrSB = "";

      String msgStrSA = "";
      msgStrSA = String(valTemperature);
      byte arrSizeSA = msgStrSA.length() + 1;
      char msgSA[arrSizeSA];
      msgStrSA.toCharArray(msgSA, arrSizeSA);
      klient.publish(topic_suhualat, msgSA);
      msgStrSA = "";

      String msgStrDT = "";
      msgStrDT = String(valDetakJantung);
      byte arrSizeDT = msgStrDT.length() + 1;
      char msgDT[arrSizeDT];
      msgStrDT.toCharArray(msgDT, arrSizeDT);
      klient.publish(topic_detak, msgDT);
      msgStrDT = "";

      String msgStrKP = "";
      msgStrKP = String(valkipas);
      byte arrSizeKP = msgStrKP.length() + 1;
      char msgKP[arrSizeKP];
      msgStrKP.toCharArray(msgKP, arrSizeKP);
      klient.publish(topic_kipas, msgKP);
      msgStrKP = "";
      
      String msgStrLP = "";
      msgStrLP = String(valdim);
      byte arrSizeLP = msgStrLP.length() + 1;
      char msgLP[arrSizeLP];
      msgStrLP.toCharArray(msgLP, arrSizeLP);
      klient.publish(topic_lampu, msgLP);
      msgStrLP = "";

      delay(50);
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  String termes = "";
  long NOW = millis();
  long LAST_SENT = 0;

  for (int i = 0; i < length; i++) {
    termes += (char)payload[i];
  }
  if (termes == "EROR"){}

  else if (termes == "kipason") {valkipas = 1; sender();}
  else if (termes == "kipasoff") {valkipas = 0; sender();}
  else if (termes == "autofan") {valkipas = 2; sender();}
  else if (termes == "lampu0") {valdim = 128; sender();}
  else if (termes == "lampu20") {valdim = 100; sender();}
  else if (termes == "lampu40") {valdim = 75; sender();}
  else if (termes == "lampu60") {valdim = 45; sender();}
  else if (termes == "lampu80") {valdim = 25; sender();}
  else if (termes == "lampu100") {valdim = 0; sender();}
  else if (termes == "autolamp") {valdim = 200; sender();}

  else { namapasien = termes;}
}

void reconnect() {
  while (!klient.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    String klientId = "ESP8266Client-";
    klientId += String(random(0xffff), HEX);
    if (klient.connect(klientId.c_str())) {
      Serial.println("Terhubung");
      klient.subscribe(topic_in);
    } 
    else {
      Serial.print("GAGAL !!!, rc =");
      Serial.print(klient.state());
      Serial.println(" Ulangi dalam 5 detik lagi...");
      delay(5000);
    }
  }
}

void loop() {

  database();
  
  kirim();
  
  if (!klient.connected()) { reconnect(); } klient.loop();
  
  spliter();

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
      
      valDetakJantung = data[1].toInt();
      int valtemp = data[2].toInt();
      int valsuhu = data[3].toInt();
      valkipas = data[4].toInt();
      valdim = data[5].toInt();

      valTemperature = valtemp;
      valSuhuBayi = valsuhu;
      
      split = false;
      sdata = "";
    }
  }
}

void sender(){
    
    String kirim = "";
    kirim += "#";
    kirim += valDetakJantung;
    kirim += "#";
    kirim += valTemperature;
    kirim += "#";
    kirim += valSuhuBayi;
    kirim += "#";
    kirim += valkipas;
    kirim += "#";
    kirim += valdim;
    kirim += "#";
    kirim += "$";

    Serial.println(kirim);
}