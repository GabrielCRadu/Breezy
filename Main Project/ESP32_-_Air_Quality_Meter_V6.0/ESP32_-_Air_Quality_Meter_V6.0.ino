/*
                                  ....
                               ^P&&&&&B7
                              ?@@G!^^J&@B.
                              ?Y?     J@@!    :~!!^.
                   ..................!&@&.  J&@&&&@&G:
                  ?&&&&&&&&&&&&&&&&&&@&5.  P@@?. .^#@&:
                  .^^^^^^^^^^^^^^^^^^.     :^:     Y@@~
               .~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~7P@@P
               ~&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&#5^
                .................................
   .!Y55555555555J^     ?PPPPPPPPPPPPPPPPPPPPY!.
  .&@@@@@@@@@@@@@@@Y    ?GGGGGGGGGGGGGGGGGGGG&@&^
  !@@@@@@@@@@@@@@@@#                    ?PY:.7@@Y
  !@@@@@@@BG&@@@@@@#                    ^#@&&@@G.
  !@@@@@@B. ~@@@@@@B
  !@@@@@@@#B&@@@@#Y. .5BBBBBBBBBBB5. 7GBBBBBBBBBBG~ .PBBBBBBBBBBBY  ?GBBBBBBBBBBG^ :PBBB7   5BBBY
  !@@@@@@@@@@&5~.    7@@@@@@&@@@@@@~ &@@@@@&&@@@@@B J@@@@@&&@@@@@@^ B&&&&&&&@@@@@P Y@@@@P   &@@@@:
  !@@@@@@@@@@@BY~.   7@@@@&. :&@@@@^ &@@@@? .5@@@@# J@@@@&. ^@@@@@^       :5@@@@5. Y@@@@P   &@@@@:
  !@@@@@@@PY#@@@@@B^ 7@@@@B   !JJJ!  &@@@@&Y5&@@@@# J@@@@@PJB@@@@@^      7&@@@G:   Y@@@@P   &@@@@:
  !@@@@@@B. !@@@@@@# 7@@@@B          &@@@@@@@@@@@@5 J@@@@@@@@@@@@&:    ^B@@@#~     Y@@@@B. ^@@@@@:
  !@@@@@@@&&@@@@@@@# 7@@@@B          &@@@@!.......  J@@@@B:.......   :G@@@&7       Y@@@@@&&@@@@@@:
  ~@@@@@@@@@@@@@@@@# 7@@@@B          &@@@@?^^^^^^^: J@@@@#^^^^^^^^  J@@@@&7^^^^^^: :PBBBBBBB@@@@@:
  .B@@@@@@@@@@@@@@&7 !@@@@G          #@@@@@@@@@@@@G 7@@@@@@@@@@@@&: &@@@@@@@@@@@@P         .&@@@@:
    :~!!!!!!!!!!!^.   ^!!!^          .~!!!!!!!!!!~.  ^!!!!!!!!!!!:  .~!!!!!!!!!!~. ~GGGGGGGG@@@@@:
                                                                                   ~&&&&&&&&&&&&B.
                                                                                     ..........
*/
#include <Wire.h>                             //Librarie pentru I2C folosita in comunicarea cu senzorul BME280
#include "FT62XXTouchScreen.h"                //Librarie pt touch (https://github.com/seaniefs/WT32-SC01-Exp)
#include "Free_Fonts.h"                       //Librarie cu fonturi pentru afisare
#include <TFT_eSPI.h>                         //Librarie pentru LCD

/*<<<<<<<<<<<<<<<<<<<<OTA>>>>>>>>>>>>>>>>>>>>*/
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
/*<<<<<<<<<<<<<<<<<<<<OTA>>>>>>>>>>>>>>>>>>>>*/


/*<<<<<<<<<<<<<<<<THINGSPEAK>>>>>>>>>>>>>>>>>*/
#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
bool TSOn = true;
int ReadIndex = 1;

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
int TSWriteDelay = 20 * 1000;                 //(20 sec)
int TSReadDelay = 20 * 1000;                  //(20 sec)
unsigned long LastTSRead, LastTSWrite;        //Variabile folosite pentru scrierea/citirea la un interval de timp a valorilor pe/de pe ThingSpeak.
int TSK = 1;                                  //Folosit pentru citirea rand pe rand a valorilor(reduce timpii morti)
/*<<<<<<<<<<<<<<<<THINGSPEAK>>>>>>>>>>>>>>>>>*/

int buzzer = 27;                             //Pin Buzzer
bool SoundOn;                                //Variabila care activeaza sunetul

int Cfinal;                                  //Calificativul ce se afiseaza pe ecran in urma calculelor

//BME280 - Temperatura, Umiditate, Presiune atmosferica//
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
Adafruit_BME280 bme;                          //Conexiunea cu senzorul se face prin I2C
float Temperatura, Umiditate, Presiune;       //Variabile ce stocheaza valorile de la senzorul BME280
int LimitaT = 35, LimitaU = 65;               //Valori limita temperatura si umiditate, pentru alarma, pot fi modificate din setari
float tmax, umax, pmax;                       //Variabile ce contin cea mai MARE valoare inregistrata a temperaturii, umiditatii si presiunii atmosferice
float tmin = 100, umin = 100, pmin = 1500;    //Variabile ce contin cea mai MICA valoare inregistrata a temperaturii, umiditatii si presiunii atmosferice(se seteaza valori initiale)
int Ctemp, Cumid;                             //Calificative pentru umiditate si temperatura

float ratioHpaMmHg = 0.75006157584566;        //Raportul dintre hPa si mmHg
int BmeUM = 2;                                //Aceasta variabila se foloseste pentru a stabili ce unitate de masura se foloseste pentru afisarea presiunii atmosferice(1-hPa; 2-MmHg)
//BME280 - Temperatura, Umiditate, Presiune atmosferica//

//MQ2 - Gaze volatile//

// https://github.com/miguel5612/MQSensorsLib/blob/master/examples/MQ-2/MQ-2.ino
//Include the library
#include "MQUnifiedsensor.h"
/*********Hardware Related Macros*************/
#define Board ("ESP-32")          // Wemos ESP-32 or other board, whatever have ESP32 core.
#define Pin (34)                  //IO25 for your ESP32 WeMos Board, pinout here: https://i.pinimg.com/originals/66/9a/61/669a618d9435c702f4b67e12c40a11b8.jpg
/********Software Related Macros*************/
#define Type ("MQ-2")             // MQ3 or other MQ Sensor, if change this verify your a and b values.
#define Voltage_Resolution (5)    // 3V3 <- IMPORTANT. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define ADC_Bit_Resolution (12)   // ESP-32 bit resolution. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define RatioMQ2CleanAir (9.83)   // RS / R0 = 9.83 ppm

float VOCvalue, VOCmax, VOCmin = 1000;           //Variabile MQ-2
int TimpIncalzireMQ2 = 5 * 60000;                       //Timp de asteptare pana se incalzeste senzorul de gaze volatile (5 minute)
int LimitaVOC = 40;                                     //Valoare limita VOC, pentru alarma, poate fi modificata din setari
float Cvoc;                                             //Calificative pentru umiditate si temperatura

/**********Globals****************/
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/**********Globals****************/

//MQ2 - Gaze volatile//


//Senzor praf//
/*
  GP2Y1010AU0F Data sheet
  http://www.sharp-world.com/products/device-china/lineup/data/pdf/datasheet/gp2y1010au_e.pdf (English)

  GP2Y1010AU0F Application note
  http://www.sharp-world.com/products/device-china/lineup/data/pdf/datasheet/gp2y1010au_appl_e.pdf (English)
*/

//#include "GP2Y1010_DustSensor.h"

const int led_pin = 4;
const int analog_pin = 33;
//GP2Y1010_DustSensor dustsensor;

float CPraf;
float CPrafMin = 10000;
float CPrafMax;
float LimitaPraf = 0.1;
//Senzor praf//

//Senzor Auxiliar//
float ETemp, ETempMin = 10000, ETempMax;
float EUmid, EUmidMin = 10000, EUmidMax;
float EPres, EPresMin = 10000, EPresMax;
//Senzor Auxiliar//

#include "Imagini.h"        //Includem fisierul cu imagini
//Convertor imagine in hex:  http://rinkydinkelectronics.com/t_imageconverter565.php

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

//ButoaneFizice//  -   se definesc si niste butoane fizice pentru navigarea prin meniuri
int ButonSus = 17;                            //Pin buton fizic "SUS"
int ButonJos = 2;                             //Pin buton fizic "JOS"

//======================ECRAN========================//
int TFT_BL = 23;                                               //Pin iluminare de fundal ecran
int luminozitate = 128;
FT62XXTouchScreen touchScreen = FT62XXTouchScreen(320, 18, 19);//(DISPLAY_HEIGHT, PIN_SDA, PIN_SCL);       //Initializare touch

//Variabile pentru aranjare rapida a textului pe ecran

int ypos1 = 50;
int ypos2 = 90;
int ypos3 = 130;
int ypos4 = 170;
int ypos5 = 210;

bool ActualizareMeniu;                        //Folosita pentru actualizarea imediata a ecranului
int Menu = 1;                                 //Variabila ce contine valoarea numerica corespunzatoare meniului afisat(se seteaza initial la 1, ca la pornire sa se afiseze meniul "HOME")
int VariabilaSelectata;
int SursaSelectata = 1;

//Variabile pt refresh valori ecran//
unsigned long TimpAnteriorSec = 0;            //Variabila folosita pentru stocarea timpului in milisecunde la care s-a intrat ultima data in bucla de o secunda
unsigned long TimpAnteriorMin = 0;            //Variabila folosita pentru stocarea timpului in milisecunde la care s-a intrat ultima data in bucla de un minut
unsigned int RefreshSecunda = 1000;           //Variabila ce stocheaza durata in milisecunde la care se face refresh valorilor afisate in meniurile "HOME", "MINIM", "MAXIME" (este setata la 1000 de milisecunde = 1 secunda)
unsigned int RefreshMinut = 60 * 1000;            //Variabila ce stocheaza durata in milisecunde la care se face refresh ceasului (este setata la 60000 de milisecunde = 1 minut)
int Ore = 12;                                 //Variabila ce stocheaza ora
int Minute = 30;                              //Variabila ce stocheaza minutul

//Variabile pt refresh valori ecran//



int TextSize = 1;                             //Se seteaza marimea fontului pentru afisare la 1

TFT_eSPI_Button key[16];                      //Se invoca clasa butanelor a librariei TFT_eSPI [se seteaza numarul de butoane dorite (8), o valoare mai mare decat cea a butoanelor ce se definesc mai jos nu influenteza functionarea programului]

void setup() {

  pinMode(ButonSus, INPUT_PULLUP);            //Se seteaza pinul ButonSus ca INPUT_PULLUP
  pinMode(ButonJos, INPUT_PULLUP);            //Se seteaza pinul ButonJos ca INPUT_PULLUP
  pinMode(TFT_BL, OUTPUT);                    //Se seteaza pinul Backlight-ului ecranului ca iesire(OUTPUT)
  digitalWrite(TFT_BL, 128);                  //Se seteaza luminozitatea ecranului la 128 din 255 (aproximativ 50%)
  pinMode(buzzer, OUTPUT);                    //Se seteaza pinul buzzer-ului ca iesire(OUTPUT)
  Serial.begin(115200);                       //Se initializeaza comunicarea pe serial

  tft.init();                                 //Se initializeaza afisajul
  touchScreen.begin();                        //Se incepe functionarea touch-ului
  tft.setTextSize(TextSize);                  //Se inlocuieste in librarie marimea textului cu cea setata precedent in variabila TextSize
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(FF6);                       //Se seteaza fontul pentru afisare

  tft.setRotation(1);                         //Se seteaza orientarea ecranului la 1(peisaj)
  tft.pushImage(0, 0, 480, 320, StartupLogo);
  tft.drawString("V6.0", 450, 5, 2);

  //THINGSPEAK//

  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network-

  //THINGSPEAK//

  delay(3000);

  /*<<<<<<<<<<<<<<<<<<<<OTA>>>>>>>>>>>>>>>>>>>>*/

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*<<<<<<<<<<<<<<<<<<<<OTA>>>>>>>>>>>>>>>>>>>>*/

  ThingSpeakRead();

  tft.fillScreen(TFT_BLACK);                  //Se umple ecranul cu negru pentru a sterge orice reziduri


  /*
      Butoanele se Initializeaza astfel:
        key[numar buton].initButtonUL(&tft, Pozitie pe x, Pozitie pe y, Latime, Inaltime, Culoare rama, Culoare buton, Culoare text, Textul de pe buton, Marime text);
        ! INITIALIZAREA NU AFISEAZA AUTOMAT BUTOANELE, ACESTEA SUNT AFISATE CU FUNCTIA: key[numar buton].drawButton(); !
  */
  key[1].initButtonUL(&tft, 4, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Home", TextSize);

  key[2].initButtonUL(&tft, 123, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Minim", TextSize);

  key[3].initButtonUL(&tft, 242, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Maxim", TextSize);

  key[4].initButtonUL(&tft, 361, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Setari", TextSize);

  key[5].initButtonUL(&tft, 5, 115, 50, 50, TFT_WHITE, TFT_WHITE, TFT_BLACK, "-", TextSize);
  key[6].initButtonUL(&tft, 425, 115, 50, 50, TFT_WHITE, TFT_WHITE, TFT_BLACK, "+", TextSize);

  key[7].initButtonUL(&tft, 5, 175, 50, 50, TFT_WHITE, TFT_WHITE, TFT_BLACK, "<", TextSize);
  key[8].initButtonUL(&tft, 425, 175, 50, 50, TFT_WHITE, TFT_WHITE, TFT_BLACK, ">", TextSize);

  //BME280//

  bool status;
  status = bme.begin();
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");//Daca nu se poate realiza conexiunea cu senzorul BME280 pe serial placa transmite un mesaj

  }

  //BME280//

  //MQ-2//
  //pinMode(MQ2, INPUT);      //definim pinul la care este legat senzorul MQ2 ca intrare

  //Set math model to calculate the PPM concentration and the value of constants
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(574.25); MQ2.setB(-2.222); // Configurate the ecuation values to get LPG concentration
  /*
    Exponential regression:
    Gas    | a      | b
    H2     | 987.99 | -2.162
    LPG    | 574.25 | -2.222
    CO     | 36974  | -3.109
    Alcohol| 3616.1 | -2.675
    Propane| 658.71 | -2.168
  */

  /**********  MQ Init ***************/
  //Remarks: Configure the pin of arduino as input.
  /****************************/
  MQ2.init();

  //If the RL value is different from 10K please assign your RL value with the following method:
  MQ2.setRL(10);// Rezistenta de pe senzorul MQ-2

  /**********  MQ CAlibration ***************/
  // Explanation:
  // In this routine the sensor will measure the resistance of the sensor supposing before was pre-heated
  // and now is on clean air (Calibration conditions), and it will setup R0 value.
  // We recomend execute this routine only on setup or on the laboratory and save on the eeprom of your arduino
  // This routine not need to execute to every restart, you can load your R0 if you know the value
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i ++)
  {
    MQ2.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply");
    while (1);
  }
  /**********  MQ CAlibration ***************/
  //MQ-2//

  //Dust Sensor - begin//
  pinMode(led_pin, OUTPUT);
  //Dust Sensor -  begin//

  Meniu1(); //Se afiseaza pe ecran meniul "HOME"
}


/**VOID LOOP**/
void loop() {

  ArduinoOTA.handle();

  if (WiFi.status() != WL_CONNECTED  && TSOn)tft.pushImage(5, 0, 30, 30, NotConnected2Wifi);

  else if (TSOn)tft.pushImage(5, 0, 30, 30, Connected2Wifi);
  else  tft.pushImage(5, 0, 30, 30, WifiOff);

  TouchPoint touchPos = touchScreen.read();

  if (touchPos.touched) {
    //Serial.printf("X: %d, Y: %d\n", touchPos.xPos, touchPos.yPos);

    for (uint8_t b = 0; b < 15; b++) {                                    //Se verifica daca s-a atins un buton
      // if (pressed && key[b].contains(touchPos.xPos, touchPos.yPos)) {
      if (key[b].contains(touchPos.xPos, touchPos.yPos)) {
        key[b].press(true);                                               //Butonul apasat va avea starea .press adevarata
      } else {
        key[b].press(false);                                              //Butonul apasat va avea starea .press falsa
      }
    }

    Butoane();                                                            //Decide ce buton afisat este apasat in functie de starea .press determinata anterior
  }
  ButoaneFizice();                                                        //Verifica apasarea butoanelor fizice

  if (millis() - TimpAnteriorMin >= RefreshMinut) {                       //Bucla se ruleaza o data pe minut
    TimpAnteriorMin = millis();                                         //Se actualizeaza valoarea contorului astfel incat se va intra in bucla doar peste un minut
    Minute++;                                                           //Se incrementeaza minutele
    if (Minute >= 60)                                                 //Cand se ajunge la 60 de minute se incrementeaza ora si se pun minutele pe 0 din nou
    {
      Ore++;
      Minute = 0;
    }
    if (Ore >= 24)                                                    //Cand ora ajunge la 24 se reseteaza la 0 si se sterge portiunea de ecran
    {
      Ore = 0;
    }
  }

  if (millis() - TimpAnteriorSec >= RefreshSecunda || ActualizareMeniu) { //Bucla se ruleaza o data pe secunda sau daca rularea este impusa fortata de ActualizareMeniu
    ActualizareMeniu = false;                                             //ActualizareMeniu se pune pe fals ca sa nu se intre inca o data in cazul in care ActualizareMeniu forteaza intrarea in bucla
    TimpAnteriorSec = millis();                                           //Se actualizeaza valoarea contorului astfel incat se va intra in bucla doar peste o secunda

    tft.setTextColor(TFT_BLACK, TFT_WHITE);

    tft.fillRect(200, 5, 80, 25, TFT_WHITE);
    tft.textdatum = TR_DATUM;                                             //Se schimba orientarea textului
    tft.drawNumber(Ore, 235, 5);                                          //Se afiseaza in coltul de sus al ecranului ora
    tft.textdatum = TC_DATUM;                                             //Se schimba orientarea textului
    tft.drawString(":", 240, 5);                                          //Se scrie pe ecran simbolul ":" intre ore si minute
    tft.textdatum = TL_DATUM;                                             //Se schimba orientarea textului
    tft.drawNumber(Minute, 250, 5);                                       //Se afiseaza pe ecran minutul

    SenzorMQ2();                                                          //Cheama bucla SenzorMQ2    // Se citesc
    SenzorBME280();                                                       //Cheama bucla BME280       // valorile.
    Senzor_Praf();
    Calificativ();                                                        //Calculare calificativ pe baza datelor citite inainte
    ThingSpeakWrite();



    ThingSpeakRead(); ReadIndex++;
    if (ReadIndex > 3)ReadIndex = 1;

    if (SoundOn) SoundOn = false;                                         // Se inverseazaza o data pe secunda starea variabilei ce determina buzzerul sa sune,
    else SoundOn = true;                                                  //                    ca sa ceeeze un efect sonor de clipire.




    // HOME //
    if (Menu == 1) {                                                      //Cand meniul 1"HOME" este selectat
      //Cand una dintre valori intrece limita setata si sunetul este pornit se seteaza pinul 25 la HIGH si valoarea respectiva se scrie cu rosu//
      //Se folosesc functiile - tft.fillRect(Pozitie pe x, Pozitie pe y, latime, inaltime, culoare); - pentru refresh rapid al valorilor
      tft.textdatum = TC_DATUM;
      tft.setTextColor(TFT_WHITE, TFT_BLACK);                             //Se seteaza initial culoarea textului la alb cu fundal negru

      if (SursaSelectata == 1) {
        if (Temperatura >= LimitaT && SoundOn)tft.setTextColor(TFT_RED, TFT_BLACK);
        else tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.fillRect(220, ypos1, 40, 20, TFT_BLACK);
        tft.drawFloat(Temperatura, 1, 240, ypos1, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        if (Umiditate >= LimitaU && SoundOn)tft.setTextColor(TFT_RED, TFT_BLACK);
        else tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.drawFloat(Umiditate, 1, 240, ypos2, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.fillRect(190, ypos3, 100, 20, TFT_BLACK);
        tft.drawFloat(Presiune, 1, 240, ypos3, 4);

        if (VOCvalue > LimitaVOC && SoundOn)tft.setTextColor(TFT_RED, TFT_BLACK);
        else tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.fillRect(180, ypos4, 120, 20, TFT_BLACK);
        tft.drawFloat(VOCvalue, 2, 240, ypos4, 4);
        //Serial.print(VOCvalue);

        if (CPraf > LimitaPraf && SoundOn)tft.setTextColor(TFT_RED, TFT_BLACK);
        else tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.fillRect(180, ypos5, 120, 20, TFT_BLACK);
        tft.drawFloat(CPraf, 3, 240, ypos5, 4);





        /*
                if (Cfinal == 10)tft.setTextColor(TFT_GREEN, TFT_BLACK);              //(x, y, w, h, radius, color)
                else if (Cfinal == 7)tft.setTextColor(TFT_YELLOW, TFT_BLACK);       //(x, y, w, h, radius, color)
                else if (Cfinal == 4)tft.setTextColor(TFT_ORANGE, TFT_BLACK);    //(x, y, w, h, radius, color)
                else if (Cfinal == 1)tft.setTextColor(TFT_RED, TFT_BLACK);     //(x, y, w, h, radius, color)
                //(415, 5, 60, 60, 10, TFT_RED);
                tft.textdatum = MC_DATUM;
                tft.drawNumber(Cfinal, 15, 50, 4);
        */
        tft.setTextColor(TFT_BLACK);
        tft.textdatum = TL_DATUM;
      }//if(SursaSelectata==1)

      if (SursaSelectata == 2) {

        tft.setTextColor(TFT_BLUE, TFT_BLACK);                         //Se seteaza initial culoarea textului la alb cu fundal negru
        tft.textdatum = TC_DATUM;

        tft.fillRect(220, ypos1, 40, 20, TFT_BLACK);
        tft.drawFloat(ETemp, 2, 240, ypos1, 4);

        tft.drawFloat(EUmid, 2, 240, ypos2, 4);

        tft.fillRect(190, ypos3, 100, 20, TFT_BLACK);
        tft.drawFloat(EPres, 2, 240, ypos3, 4);


        tft.textdatum = TL_DATUM;
      }//if(SursaSelectata==2)
    }
    if ((Temperatura >= LimitaT || Umiditate >= LimitaU || (VOCvalue > LimitaVOC && TimpIncalzireMQ2 < millis()) || CPraf > LimitaPraf) && SoundOn) digitalWrite(buzzer, HIGH);
    else digitalWrite(buzzer, LOW);

    // HOME //


    // MINIM //
    if (Menu == 2) {                                                  //Cand meniul 2"MINIM" este selectat
      //Se afiseaza valorile minime inregistrate de la pornirea placii, se face refresh cu tft.fillRect();
      if (SursaSelectata == 1) {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);                         //Se seteaza initial culoarea textului la alb cu fundal negru
        tft.textdatum = TC_DATUM;

        tft.fillRect(220, ypos1, 40, 20, TFT_BLACK);
        tft.drawFloat(tmin, 2, 240, ypos1, 4);

        tft.drawFloat(umin, 2, 240, ypos2, 4);

        tft.fillRect(190, ypos3, 100, 20, TFT_BLACK);
        tft.drawFloat(pmin, 2, 240, ypos3, 4);

        tft.fillRect(180, ypos4, 120, 20, TFT_BLACK);
        tft.drawFloat(VOCmin, 2, 240, ypos4, 4);

        tft.fillRect(180, ypos5, 120, 20, TFT_BLACK);
        tft.drawFloat(CPrafMin, 3, 240, ypos5, 4);

        tft.textdatum = TL_DATUM;
      }

      if (SursaSelectata == 2) {
        tft.setTextColor(TFT_BLUE, TFT_BLACK);                         //Se seteaza initial culoarea textului la alb cu fundal negru
        tft.textdatum = TC_DATUM;

        tft.fillRect(200, ypos1, 80, 20, TFT_BLACK);
        tft.drawFloat(ETempMin, 2, 240, ypos1, 4);

        tft.drawFloat(EUmidMin, 2, 240, ypos2, 4);

        tft.fillRect(190, ypos3, 100, 20, TFT_BLACK);
        tft.drawFloat(EPresMin, 2, 240, ypos3, 4);

        tft.textdatum = TL_DATUM;
      }
    }
    // MINIM //


    // MAXIME //
    if (Menu == 3) {                                                  //Cand meniul 3"MAXIM" este selectat
      //Se afiseaza valorile maxime inregistrate de la pornirea placii, se face refresh cu tft.fillRect();

      if (SursaSelectata == 1) {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);                         //Se seteaza initial culoarea textului la alb cu fundal negru
        tft.textdatum = TC_DATUM;

        tft.fillRect(200, ypos1, 80, 20, TFT_BLACK);
        tft.drawFloat(tmax, 2, 240, ypos1, 4);

        tft.drawFloat(umax, 2, 240, ypos2, 4);

        tft.fillRect(190, ypos3, 100, 20, TFT_BLACK);
        tft.drawFloat(pmax, 2, 240, ypos3, 4);

        tft.fillRect(180, ypos4, 120, 20, TFT_BLACK);
        tft.drawFloat(VOCmax, 2, 240, ypos4, 4);

        tft.fillRect(180, ypos5, 120, 20, TFT_BLACK);
        tft.drawFloat(CPrafMax, 3, 240, ypos5, 4);

        tft.textdatum = TL_DATUM;
      }

      if (SursaSelectata == 2) {
        tft.setTextColor(TFT_BLUE, TFT_BLACK);                         //Se seteaza initial culoarea textului la alb cu fundal negru
        tft.textdatum = TC_DATUM;

        tft.fillRect(200, ypos1, 80, 20, TFT_BLACK);
        tft.drawFloat(ETempMax, 2, 240, ypos1, 4);

        tft.drawFloat(EUmidMax, 2, 240, ypos2, 4);

        tft.fillRect(190, ypos3, 100, 20, TFT_BLACK);
        tft.drawFloat(EPresMax, 2, 240, ypos3, 4);

        tft.textdatum = TL_DATUM;
      }

    }
    // MAXIME //

    // SETARI //
    if (Menu == 4) {                                                  //Cand meniul 4"SETARI" este selectat
      tft.setTextColor(TFT_WHITE, TFT_BLACK);                         //Se seteaza initial culoarea textului la alb cu fundal negru
      tft.fillRect(55, 130, 370, 100, TFT_BLACK);                     //Se face refresh la mijlocul ecranului unde sunt afisate valori, nume variabile selectate


      if (VariabilaSelectata == 0) {                                  //Valoarea 0 a VariabilaSelectata corespunde variabilei ce retine numarul indicator al sursei active
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Selecteaza sursa", 240, 195, 4);              //Se scrie pe ecran ce variabila se modifica (numarul indicator al sursei active)
        tft.drawNumber(SursaSelectata, 240, 130);                     //Se afiseaza valoarea variabilei ce retine numarul indicator al sursei active
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }

      if (VariabilaSelectata == 1) {                                  //Valoarea 1 a VariabilaSelectata corespunde setarii orei
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Ore", 240, 195, 4);                           //Se scrie pe ecran ce variabila se modifica (Orele)
        tft.drawNumber(Ore, 240, 130);                                //Se afiseaza valoarea variabilei ce retine orele
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }

      if (VariabilaSelectata == 2) {                                  //Valoarea 2 a VariabilaSelectata corespunde setarii minutelor
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Minute", 240, 195, 4);                        //Se scrie pe ecran ce variabila se modifica (Minutele)
        tft.drawNumber(Minute, 240, 130);                             //Se afiseaza valoarea variabilei ce retine minutele
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }

      if (VariabilaSelectata == 3) {                                  //Valoarea 3 a VariabilaSelectata corespunde setarii valorii la care alerta de temperatura se declanseaza
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Alerta Temperatura", 240, 195, 4);            //Se scrie pe ecran ce variabila se modifica (Alerta Temperatura)
        tft.drawNumber(LimitaT, 240, 130);                            //Se afiseaza valoarea setata a alertei
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }

      if (VariabilaSelectata == 4) {                                  //Valoarea 4 a VariabilaSelectata corespunde setarii valorii la care alerta de umiditate se declanseaza
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Alerta Umiditate", 240, 195, 4);              //Se scrie pe ecran ce variabila se modifica (Alerta Umiditate)
        tft.drawNumber(LimitaU, 240, 130);                            //Se afiseaza valoarea setata a alertei
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }

      if (VariabilaSelectata == 5) {                                  //Valoarea 5 a VariabilaSelectata corespunde setarii valorii la care alerta de Compusi Organici Volatili se declanseaza
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Alerta VOC", 240, 195, 4);                    //Se scrie pe ecran ce variabila se modifica (Alerta VOC)
        tft.drawNumber(LimitaVOC, 240, 130);                          //Se afiseaza valoarea setata a alertei
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }

      if (VariabilaSelectata == 6) {                                  //Valoarea 5 a VariabilaSelectata corespunde setarii valorii la care alerta de Compusi Organici Volatili se declanseaza
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Alerta Praf", 240, 195, 4);                   //Se scrie pe ecran ce variabila se modifica (Alerta VOC)
        tft.drawFloat(LimitaPraf, 2, 240, 130, 4);                    //Se afiseaza valoarea setata a alertei
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }

      if (VariabilaSelectata == 7) {                                  //Valoarea 6 a VariabilaSelectata corespunde setarii valorii la care alerta de Compusi Organici Volatili se declanseaza
        tft.textdatum = TC_DATUM;                                     //Se schimba alinierea textului la Top_Centre(se scrie de sus in jos, centrat pe orizontala fata de coordonata data)
        tft.drawString("Scriere/Citire ThingSpeak", 240, 195, 4);     //Se scrie pe ecran ce variabila se modifica (Alerta VOC)

        if (TSOn) {
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.drawString("Pornit", 240, 130, 4);
        }

        if (!TSOn) {
          tft.setTextColor(TFT_RED, TFT_BLACK);
          tft.drawString("Oprit", 240, 130, 4);
        }
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.textdatum = TL_DATUM;                                     //Se schimba alinierea textului la Top_Left(se scrie de sus in jos, de la stanga spre dreapta fata de coordonata data)
      }
    }
    // SETARI //
  }//   SFARSIT Bucla de 1 secunda   //
}//   SFARSIT VOID LOOP   //



//SENZORI//

void SenzorBME280() {

  Temperatura = bme.readTemperature();                    //Citire valoarea temperaturii de la BME280
  Presiune = bme.readPressure() / 100.0F;                 //Citire valoarea presiunii atmosferice de la BME280 si transformare din Pa in HPa
  if (BmeUM == 2)Presiune = Presiune * ratioHpaMmHg;             //Se transforma valoarea "Presiune" din hPa in mmHg daca este setat in program
  Umiditate = bme.readHumidity();                         //Citire valoarea umiditatii de la BME280


  if (Temperatura > tmax)tmax = Temperatura;          //Variabila tmax ramane cu valoarea celei mai mari temperaturi inregistrate
  if (Temperatura < tmin)tmin = Temperatura;          //Variabila tmin ramane cu valoarea celei mai mici temperaturi inregistrate

  if (Umiditate > umax)umax = Umiditate;              //Variabila umax ramane cu cea mai mare valoare inregistrata de variabila Umiditate
  if (Umiditate < umin)umin = Umiditate;              //Variabila umin ramane cu cea mai mica valoare inregistrata de variabila Umiditate

  if (Presiune > pmax)pmax = Presiune;                //Variabila pmax ramane cu cea mai mare valoare inregistrata de variabila Presiune
  if (Presiune < pmin)pmin = Presiune;                //Variabila pmin ramane cu cea mai mica valoare inregistrata de variabila Presiune
}

void SenzorMQ2() {
  VOCvalue = 0;                                    //Variabila VOCvalue este setata la 0,

  for (int i = 1; i <= 5; i++) {                          //Bucla for ce se ruleaza de 5 ori, folosita pentru a evita zgomotul din citiri rezultand o valoare stabila
    MQ2.update();                                         //Se actualizeaza valoarea citita de Esp
    VOCvalue += MQ2.readSensor();                  //Variabilei VOCvalue i se aduna 5 valori distincte citite la timpi diferiti
  } VOCvalue /= 5;         //Valoarea variabilei ConcentraGaz este impartia la 5 pentru a incheia media aritmetica, returneaza valoarea ce se afiseaza pe ecran

  //MQ2.serialDebug();                                      // Activeaza trimiterea valorior pe serial pt senzorul MQ2
  //Serial.println(VOCvalue);                      //se printeaza pe serial valoarea analogica a pinului MQ2

  if (TimpIncalzireMQ2 < millis())                        // Nu se iau in considerare valorile minime si maxime decat dupa ce se incalzeste senzorul
  {
    if (VOCvalue > VOCmax)VOCmax = VOCvalue; //Variabila VOCmax ramane cu cea mai mare valoare inregistrata de variabila VOCvalue
    if (VOCvalue < VOCmin)VOCmin = VOCvalue; //Variabila VOCmin ramane cu cea mai mica valoare inregistrata de variabila VOCvalue
  }
}

void Senzor_Praf() {  // ecuatia de calcul luata de pe: http://www.howmuchsnow.com/arduino/airquality/
  CPraf = 0;
  for (int i = 1; i <= 5; i++) {
    digitalWrite(led_pin, LOW);
    delayMicroseconds(280);                   //Sampling time

    float mesured = analogRead(analog_pin);  //read analog pin / Dust value
    delayMicroseconds(40);                    //Delta time

    digitalWrite(led_pin, HIGH);
    delayMicroseconds(9680);                  //Sleep time

    /*
      0 - 5V mapped to 0 - 1023 integer values
      // recover voltage
      calcVoltage = voMeasured * (this->_VCC / 1024.0);
      dustDensity = (0.17 * calcVoltage - 0.1) * 1000.0;
    */


    // calc dust density
    CPraf += (0.172 * (mesured * (3.3 / 4096)) ); // (0.17 * (valoare masurata * (tensiune intrare / numar biti)) - 0.1) * 1000.;
    //if ( CPraf < 0 )  CPraf = 0.;
  } CPraf /= 5;



  if (CPrafMin > CPraf)CPrafMin = CPraf;
  if (CPrafMax < CPraf)CPrafMax = CPraf;
}

void Calificativ() {
  if ((Temperatura >= 18) && (Temperatura <= 26)) Ctemp = 1;
  else Ctemp = 0;

  if ((Umiditate >= 30) && (Umiditate <= 50)) Cumid = 1; //daca umiditatea este intre 30% si 50% calificativul este 10
  else Cumid = 0;

  if (VOCvalue <= 15) Cvoc = 1;
  else Cvoc = 0;

  Cfinal = (Ctemp + Cumid + Cvoc) * 3 + 1;
}



void ThingSpeakWrite() {
  if (TSOn  && (millis() - LastTSWrite >= TSWriteDelay)) {
    LastTSWrite = millis();
    ThingSpeak.setField(1, Temperatura);
    ThingSpeak.setField(2, Presiune);
    ThingSpeak.setField(3, Umiditate);
    if (TimpIncalzireMQ2 < millis())ThingSpeak.setField(4, VOCvalue);

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    ThingSpeak.setField(5, CPraf);
  }
}

void ThingSpeakRead() {
  if (TSOn && (millis() - LastTSRead >= TSReadDelay)) {
    LastTSRead = millis();

    if (ReadIndex == 1) {
      ETemp = ThingSpeak.readFloatField(1748573, 1);
      if (ETemp > ETempMax)ETempMax = ETemp;
      if (ETemp < ETempMin)ETempMin = ETemp;
    }

    if (ReadIndex == 2) {
      EUmid = ThingSpeak.readFloatField(1748573, 2);
      if (EUmid > EUmidMax)EUmidMax = EUmid;
      if (EUmid < EUmidMin)EUmidMin = EUmid;
    }

    if (ReadIndex == 3) {
      EPres = ThingSpeak.readFloatField(1748573, 3);
      if (EPres > EPresMax)EPresMax = EPres;
      if (EPres < EPresMin)EPresMin = EPres;
    }
  }
}

//BUTOANE//

void ButoaneFizice() {                                    //Bucla ce se ocupa cu butoanele fizice
  bool ButonSusApasat = !digitalRead(ButonSus);           //Citire si stocare stare ButonSus
  bool ButonJosApasat = !digitalRead(ButonJos);           //Citire si stocare stare ButonJos

  if (ButonSusApasat) {                                   //Daca ButonSus este apasat se ruleaza↓
    Menu++;                                               //Se incrementeaza variabila Menu
    if (Menu > 4) Menu = 1;                               //Pentru ca avem doar 4 meniuri vrem ca sa trecem din meniul 4("SETARI") inapoi in meniul 1("HOME")
    ActualizareMeniu = true;                              //Actualizare fortata meniu
    delay(300);                                           //Delay folosit ca sa poti tine apasat pe buton, si scroll-ul prin meniuri sa nu fie prea rapid ca sa te poti opri in meniul dorit
    Meniuri();                                            //Se cheama bucla Meniuri pentru afisarea meniului selectat
  }

  if (ButonJosApasat) {                                   //Daca ButonJos este apasat se ruleaza↓
    Menu--;                                               //Se decrementeaza variabila Menu
    if (Menu < 1) Menu = 4;                               //Vrem sa sarim inapoi in meniul 4("SETARI") daca decrementam variabila Meniu cand suntem in meniul 1("HOME")
    ActualizareMeniu = true;                              //Actualizare fortata meniu
    delay(300);                                           //Delay folosit ca sa poti tine apasat pe buton, si scroll-ul prin meniuri sa nu fie prea rapid ca sa te poti opri in meniul dorit
    Meniuri();                                            //Se cheama bucla Meniuri pentru afisarea meniului selectat
  }
}

void Butoane() {                                          //Bucla ce se ocupa cu butoanele virtuale, afisate sau nu pe ecran

  if (key[1].justPressed() && Menu != 1) {              //Daca butonul 1 este apasat si nu suntem in meniul 1 ("HOME") se ruleaza↓
    key[1].press(false);                                //Se schimba starea butonului 1 la ne-apasata
    Menu = 1; // HOME //                                   //Se seteaza variabila Menu la 1, corespunde meniului 1("HOME")
    ActualizareMeniu = true;                            //Actualizare fortata ecran
    Meniu1();                                           //Se afiseaza meniul 1("HOME")
  }


  if (key[2].justPressed() && Menu != 2) {              //Daca butonul 2 este apasat si nu suntem in meniul 2 ("MINIM") se ruleaza↓
    key[2].press(false);                                //Se schimba starea butonului 2 la ne-apasata
    Menu = 2; // MINIM //                                  //Se seteaza variabila Menu la 2, corespunde meniului 2("MINIM")
    ActualizareMeniu = true;                            //Actualizare fortata ecran
    Meniu2();                                           //Se afiseaza meniul 2("MINIM")
  }


  if (key[3].justPressed() && Menu != 3) {              //Daca butonul 3 este apasat si nu suntem in meniul 3 ("MAXIM") se ruleaza↓
    key[3].press(false);                                //Se schimba starea butonului 3 la ne-apasata
    Menu = 3; // MAXIM //                                  //Se seteaza variabila Menu la 3, corespunde meniului 3("MAXIM")
    ActualizareMeniu = true;                            //Actualizare fortata ecran
    Meniu3();                                           //Se afiseaza meniul 3("MAXIM")
  }


  if (key[4].justPressed() && Menu != 4) {              //Daca butonul 4 este apasat si nu suntem in meniul 4 ("SETARI") se ruleaza↓
    key[4].press(false);                                //Se schimba starea butonului 4 la ne-apasata
    Menu = 4; // SETARI //                                 //Se seteaza variabila Menu la 4, corespunde meniului 4("SETARI")
    ActualizareMeniu = true;                            //Actualizare fortata ecran
    Meniu4();                                           //Se afiseaza meniul 4("SETARI")
  }

  if (key[5].justPressed() && Menu == 4) {
    key[5].press(false);                            //Se schimba starea butonului 5 la ne-apasata
    ActualizareMeniu = true;                        //Actualizare fortata ecran
    //In functie de variabila este selectata la apasarea butonului aceasta se decrementeaza corespunzator tipului de date//

    if (VariabilaSelectata == 0)
    {
      SursaSelectata--;
      if (SursaSelectata < 1)SursaSelectata = 2;
    }

    if (VariabilaSelectata == 1)
    {
      Ore--;
      if (Ore < 0)Ore = 23;
    }

    if (VariabilaSelectata == 2)
    {
      Minute--;
      if (Minute < 0)Minute = 59;
    }

    if (VariabilaSelectata == 3)
    {
      LimitaT--;
      if (LimitaT < 0)LimitaT = 45;
    }

    if (VariabilaSelectata == 4)
    {
      LimitaU--;
      if (LimitaU < 0)LimitaU = 90;
    }

    if (VariabilaSelectata == 5)
    {
      LimitaVOC -= 5;
      if (LimitaVOC < 0)LimitaVOC = 400;
    }

    if (VariabilaSelectata == 6)
    {
      LimitaPraf -= 0.05;
      if (LimitaPraf < 0.05)LimitaPraf = 1;
    }

    if (VariabilaSelectata == 7)
    {
      TSOn = false;
    }
  }

  if (key[6].justPressed() && Menu == 4) {
    key[6].press(false);                             //Se schimba starea butonului 6 la ne-apasata
    ActualizareMeniu = true;                         //Actualizare fortata ecran
    //In functie de variabila este selectata la apasarea butonului aceasta se incrementeaza corespunzator tipului de date//

    if (VariabilaSelectata == 0)
    {
      SursaSelectata++;
      if (SursaSelectata > 2)SursaSelectata = 1;
    }

    if (VariabilaSelectata == 1)
    {
      Ore++;
      if (Ore > 23)Ore = 0;
    }

    if (VariabilaSelectata == 2)
    {
      Minute++;
      if (Minute > 59)Minute = 0;
    }

    if (VariabilaSelectata == 3)
    {
      LimitaT++;
      if (LimitaT > 45)LimitaT = 0;
    }

    if (VariabilaSelectata == 4)
    {
      LimitaU++;
      if (LimitaU > 90)LimitaU = 0;
    }

    if (VariabilaSelectata == 5)
    {
      LimitaVOC += 5;
      if (LimitaVOC > 400)LimitaVOC = 0;
    }

    if (VariabilaSelectata == 6)
    {
      LimitaPraf += 0.05;
      if (LimitaPraf > 1)LimitaPraf = 0.05;
    }

    if (VariabilaSelectata == 7)
    {
      TSOn = true;
    }
  }

  if (key[7].justPressed() && Menu == 4) {               //Daca se apasa butonul 7(<) si suntem in meniul 4 "SETARI" se schimba intr-o directie variabila selectata
    key[7].press(false);                             //Se schimba starea butonului 7 la ne-apasata
    ActualizareMeniu = true;                         //Actualizare fortata ecran

    VariabilaSelectata--;
    if (VariabilaSelectata < 0)VariabilaSelectata = 7;
  }

  if (key[8].justPressed() && Menu == 4) {               //Daca se apasa butonul 8(>) si suntem in meniul 4 "SETARI" se schimba in cealalta directie variabila selectata
    key[8].press(false);                             //Se schimba starea butonului 8 la ne-apasata
    ActualizareMeniu = true;                         //Actualizare fortata ecran

    VariabilaSelectata++;
    if (VariabilaSelectata > 7)VariabilaSelectata = 0;
  }
}


//MENIURI//
void Meniuri() {                                     //Aceasta bucla decide ce meniu sa se afiseze in functie de valoarea variabilei Menu
  tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
  if (Menu == 1)                                     //Pentru Menu = 1 corespunde meniul 1 "HOME"
    Meniu1();
  else if (Menu == 2)                                //Pentru Menu = 2 corespunde meniul 2 "MINIM"
    Meniu2();
  else if (Menu == 3)                                //Pentru Menu = 3 corespunde meniul 3 "MAXIME"
    Meniu3();
  else if (Menu == 4)                                //Pentru Menu = 4 corespunde meniul 4 "SETARI"
    Meniu4();

}

void Meniu1() {                                      //In meniul 1 "HOME"↓
  tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
  tft.fillRect(0, 0, 480, 30, TFT_WHITE);            //Desenare bara alba in partea de sus a ecranului
  key[1].drawButton(true);                           //Afiseaza butonul"Home"
  key[2].drawButton();                               //Afiseaza butonul"Minim"
  key[3].drawButton();                               //Afiseaza butonul"Maxim"
  key[4].drawButton();                               //Afiseaza butonul"Setari"

  //tft.pushImage(440, ypos1, 32, 21, MenuIcon);          //Buton "Alege sursa"

  tft.setTextColor(TFT_WHITE, TFT_BLACK);            //Se seteaza initial culoarea textului la alb cu fundal negru

  //Se afiseaza  textul static, deoarece nu se mai actualizeaza cresc performanta//
  tft.drawString("Temperatura", 10, ypos1, 4);
  tft.drawString("*C", 360, ypos1, 4);
  tft.drawString("Umiditate", 10, ypos2, 4);
  tft.drawString("%", 360, ypos2, 4);
  tft.drawString("Presiune", 10, ypos3, 4);
  if (BmeUM == 1)tft.drawString("hPa", 360, ypos3, 4);
  if (BmeUM == 2)tft.drawString("mmHg", 360, ypos3, 4);

  if (SursaSelectata == 1) {
    tft.drawString("VOC", 10, ypos4, 4);
    tft.drawString("PPM", 360, ypos4, 4);

    tft.drawString("Praf", 10, ypos5, 4);
    tft.drawString("mg/m^3", 360, ypos5, 4);
  }

}

void Meniu2() {                                      //In meniul 2 "MINIME"↓
  tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
  tft.fillRect(0, 0, 480, 30, TFT_WHITE);            //Desenare bara alba in partea de sus a ecranului
  key[1].drawButton();                               //Afiseaza butonul"Home"
  key[2].drawButton(true);                           //Afiseaza butonul"Minim"
  key[3].drawButton();                               //Afiseaza butonul"Maxim"
  key[4].drawButton();                               //Afiseaza butonul"Setari"
  //tft.pushImage(440, ypos1, 32, 21, MenuIcon);          //Buton "Alege sursa"

  tft.setTextColor(TFT_WHITE, TFT_BLACK);            //Se seteaza initial culoarea textului la alb cu fundal negru

  //Se afiseaza  textul static, deoarece nu se mai actualizeaza cresc performanta//
  tft.drawString("TMin", 10, ypos1, 4);
  tft.drawString("*C", 360, ypos1, 4);
  tft.drawString("UMin", 10, ypos2, 4);
  tft.drawString("%", 360, ypos2, 4);
  tft.drawString("PMin", 10, ypos3, 4);
  if (BmeUM == 1)tft.drawString("hPa", 360, ypos3, 4);
  if (BmeUM == 2)tft.drawString("mmHg", 360, ypos3, 4);

  if (SursaSelectata == 1) {
    tft.drawString("VOC Min", 10, ypos4, 4);
    tft.drawString("PPM", 360, ypos4, 4);

    tft.drawString("Praf Min", 10, ypos5, 4);
    tft.drawString("mg/m^3", 360, ypos5, 4);
  }

}

void Meniu3() {                                      //In meniul 2 "MAXIME"↓
  tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
  tft.fillRect(0, 0, 480, 30, TFT_WHITE);            //Desenare bara alba in partea de sus a ecranului
  key[1].drawButton();                               //Afiseaza butonul"Home"
  key[2].drawButton();                               //Afiseaza butonul"Minim"
  key[3].drawButton(true);                           //Afiseaza butonul"Maxim"
  key[4].drawButton();                               //Afiseaza butonul"Setari"
  //tft.pushImage(440, ypos1, 32, 21, MenuIcon);          //Buton "Alege sursa"

  tft.setTextColor(TFT_WHITE, TFT_BLACK);            //Se seteaza initial culoarea textului la alb cu fundal negru

  //Se afiseaza  textul static, deoarece nu se mai actualizeaza cresc performanta//
  tft.drawString("TMax", 10, ypos1, 4);
  tft.drawString("*C", 360, ypos1, 4);
  tft.drawString("UMax", 10, ypos2, 4);
  tft.drawString("%", 360, ypos2, 4);
  tft.drawString("PMax", 10, ypos3, 4);
  if (BmeUM == 1)tft.drawString("hPa", 360, ypos3, 4);
  if (BmeUM == 2)tft.drawString("mmHg", 360, ypos3, 4);

  if (SursaSelectata == 1) {
    tft.drawString("VOC Max", 10, ypos4, 4);
    tft.drawString("PPM", 360, ypos4, 4);

    tft.drawString("Praf Max", 10, ypos5, 4);
    tft.drawString("mg/m^3", 360, ypos5, 4);
  }

}

void Meniu4() {                                      //In meniul 4 "SETARI"↓


  tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
  tft.fillRect(0, 0, 480, 30, TFT_WHITE);            //Desenare bara alba in partea de sus a ecranului
  key[1].drawButton();                               //Afiseaza butonul"Home"
  key[2].drawButton();                               //Afiseaza butonul"Minim"
  key[3].drawButton();                               //Afiseaza butonul"Maxim"
  key[4].drawButton(true);                           //Afiseaza butonul"Setari"

  key[5].drawButton();                               //Afiseaza butonul"-"
  key[6].drawButton();                               //Afiseaza butonul"+"

  key[7].drawButton();                               //Afiseaza butonul"<"
  key[8].drawButton();                               //Afiseaza butonul">"
}

void MeniuESensor() {                                //In meniul "Senzor Auxiliar"↓
  tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
  tft.fillRect(0, 0, 480, 30, TFT_WHITE);            //Desenare bara alba in partea de sus a ecranului
  key[1].drawButton(true);                           //Afiseaza butonul"Home"
  key[2].drawButton();                               //Afiseaza butonul"Minim"
  key[3].drawButton();                               //Afiseaza butonul"Maxim"
  key[4].drawButton();                               //Afiseaza butonul"Setari"
  //tft.pushImage(240, ypos3, 30, 19, MenuIcon);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);            //Se seteaza initial culoarea textului la alb cu fundal negru

  //Se afiseaza  textul static, deoarece nu se mai actualizeaza cresc performanta//
  tft.drawString("Temperatura", 10, ypos1, 4);
  tft.drawString("*C", 360, ypos1, 4);
  tft.drawString("Umiditate", 10, ypos2, 4);
  tft.drawString("%", 360, ypos2, 4);
  tft.drawString("Presiune", 10, ypos2, 4);
  if (BmeUM == 1)tft.drawString("hPa", 360, ypos3, 4);
  if (BmeUM == 2)tft.drawString("mmHg", 360, ypos3, 4);
}
