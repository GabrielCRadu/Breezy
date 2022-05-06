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
  !@@@@@@B. ~@@@@@@B                      :!7~:                                                     
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
#include <Wire.h>
#include "FT62XXTouchScreen.h"
#include "Free_Fonts.h"                         
#include <SPI.h>                              //
#include <TFT_eSPI.h>                         //Hardware-specific library
int buzzer = 25;                              //Pin Buzzer
bool BuzzerOn;

//BME280 - Temperatura, Umiditate, Presiune atmosferica//
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
Adafruit_BME280 bme;  //foloseste I2C
float Temperatura, Umiditate, Presiune;
float tmax, umax, pmax;                       //Variabile ce contin cea mai MARE valoare inregistrata a temperaturii, umiditatii si presiunii atmosferice
float tmin = 100, umin = 100, pmin = 1500;    //Variabile ce contin cea mai MICA valoare inregistrata a temperaturii, umiditatii si presiunii atmosferice(se seteaza valori initiale)
//BME280 - Temperatura, Umiditate, Presiune atmosferica//

//MQ2 - Gaze volatile//

// https://github.com/miguel5612/MQSensorsLib/blob/master/examples/MQ-2/MQ-2.ino
// https://github.com/miguel5612/MQSensorsLib_Docs/blob/master/Docs/MQSensorLib_2.0.pdf //MANUAL
//Include the library
#include "MQUnifiedsensor.h"
/*********Hardware Related Macros*************/
#define Board ("ESP-32")  // Wemos ESP-32 or other board, whatever have ESP32 core.
#define Pin (34)          //IO25 for your ESP32 WeMos Board, pinout here: https://i.pinimg.com/originals/66/9a/61/669a618d9435c702f4b67e12c40a11b8.jpg
/********Software Related Macros*************/
#define Type ("MQ-2")             // MQ3 or other MQ Sensor, if change this verify your a and b values.
#define Voltage_Resolution (5)  // 3V3 <- IMPORTANT. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define ADC_Bit_Resolution (12)   // ESP-32 bit resolution. Source: https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
#define RatioMQ2CleanAir (9.83)   // RS / R0 = 9.83 ppm

float ConcentratieGaz, VOCmax, VOCmin = 10000;
int TimpIncalzireMQ2 = 120000;// Timp de asteptare pana se incalzeste senzorul de gaze volatile

/**********Globals****************/
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/**********Globals****************/

//MQ2 - Gaze volatile//



#include "Imagini.h"  //Includem fisierul cu imagini
// convertor imagine to hex:  http://rinkydinkelectronics.com/t_imageconverter565.php

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

//ButoaneFizice//
int ButonSus = 17;                            //Pin buton fizic "SUS"
int ButonJos = 2;                             //Pin buton fizic "JOS"

int TFT_BL = 23;                              //Pin backlight ecran

#define DISPLAY_WIDTH 480                     //Latimea Ecranului in pixeli
#define DISPLAY_HEIGHT 320                    //Inaltimea ecranului in pixeli

FT62XXTouchScreen touchScreen = FT62XXTouchScreen(DISPLAY_HEIGHT, 18, 19);//(DISPLAY_HEIGHT, PIN_SDA, PIN_SCL);       //Initializare touch

int16_t h;
int16_t w;

bool ActualizareMeniu;                        //Folosita pentru actualizarea imediata a ecranului
int Menu = 1;                                 //Variabila ce contine valoarea numerica corespunzatoare meniului afisat(se seteaza initial la 1, ca la pornire sa se afiseze meniul "HOME")

//Variabile pt refresh valori ecran//
unsigned long TimpAnteriorSec = 0;            //Variabila folosita pentru stocarea timpului in milisecunde la care s-a intrat ultima data in bucla de o secunda
unsigned long TimpAnteriorMin = 0;            //Variabila folosita pentru stocarea timpului in milisecunde la care s-a intrat ultima data in bucla de un minut
unsigned int RefreshSecunda = 1000;           //Variabila ce stocheaza durata in milisecunde la care se face refresh valorilor afisate in meniurile "HOME", "MINIM", "MAXIME" (este setata la 1000 de milisecunde = 1 secunda)
unsigned int RefreshMinut = 60000;            //Variabila ce stocheaza durata in milisecunde la care se face refresh ceasului (este setata la 60000 de milisecunde = 1 minut)
unsigned int Ore = 0;                         //Variabila ce stocheaza ora
unsigned int Minute = 0;                      //Variabila ce stocheaza minutul

//Variabile pt refresh valori ecran//


TFT_eSPI_Button key[16];                      //Se invoca clasa butanelor a librariei TFT_eSPI [se seteaza numarul de butoane dorite (16), o valoare mai mare decat cea a butoanelor ce se definesc mai jos nu influenteza functionarea programului]

int TextSize = 1;                             //Se seteaza marimea fontului pentru afisare la 1

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
  tft.setFreeFont(FF6);                       //Se seteaza fontul pentru afisare

  h = tft.height();                           //!
  w = tft.width();                            //!

  tft.setRotation(1);                         //Se seteaza orientarea ecranului la 1(peisaj)
  tft.fillScreen(TFT_BLACK);                  //Se umple ecranul cu negru pentru a sterge orice reziduri

/*
    Butoanele se Initializeaza astfel:
      key[numar buton].initButtonUL(&tft, Pozitie pe x, Pozitie pe y, Latime, Inaltime, Culoare rama, Culoare buton, Culoare text, Textul de pe buton, Marime text);

      ! INITIALIZAREA NU AFISEAZA AUTOMAT BUTOANELE, ACESTEA SUNT AFISATE CU FUNCTIA: key[numar buton].drawButton(); !
*/
  key[1].initButtonUL(&tft, 4, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Home", TextSize);  
  key[1].drawButton();

  key[2].initButtonUL(&tft, 123, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Minim", TextSize);
  key[2].drawButton();

  key[3].initButtonUL(&tft, 242, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Maxim", TextSize);
  key[3].drawButton(); 

  key[4].initButtonUL(&tft, 361, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Setari", TextSize);
  key[4].drawButton();    
  //tft.pushImage(280, 275, 40, 40, CogWheel);                                                    //Afisare iconita Setari

  //BME280//

  bool status;
  status = bme.begin();
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    //while (1);
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
  for(int i = 1; i<=10; i ++)
  {
    MQ2.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    Serial.print(".");
  }
  MQ2.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}
  /**********  MQ CAlibration ***************/ 
  //MQ-2//
  Meniu1();
}


/**VOID LOOP**/
void loop() {
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

  if (millis() - TimpAnteriorMin >= RefreshMinut){                        //Bucla se ruleaza o data pe minut
      TimpAnteriorMin = millis();                                         //Se actualizeaza valoarea contorului astfel incat se va intra in bucla doar peste un minut
      Minute++;                                                           //Se incrementeaza minutele
        if(Minute>=60)                                                    //Cand se ajunge la 60 de minute se incrementeaza ora si se pun minutele pe 0 din nou
        {
          Ore++;
          Minute=0;                                                 
        }
        if(Ore>=24)                                                       //Cand ora ajunge la 24 se reseteaza la 0 si se sterge portiunea de ecran
        {
          Ore=0; 
          tft.fillRect(0, 5, 30, 20, TFT_BLACK);
             
        }                                            
  }

  if (millis() - TimpAnteriorSec >= RefreshSecunda || ActualizareMeniu) {       //Bucla se ruleaza o data pe secunda sau daca rularea este impusa fortata de ActualizareMeniu
    ActualizareMeniu = false;                                                   //ActualizareMeniu se pune pe fals ca sa nu se intre inca o data in cazul in care ActualizareMeniu forteaza intrarea in bucla
    TimpAnteriorSec = millis();                                                 //Se actualizeaza valoarea contorului astfel incat se va intra in bucla doar peste o secunda

    tft.drawNumber(Ore,5,5);                                                    //Se afiseaza in coltul de sus al ecranului ora
    tft.drawString(":", 30, 5);                                                 //Se scrie pe ecran simbolul ":" intre ore si minute
    tft.drawNumber(Minute,40,5);                                                //Se afiseaza pe ecran minutul

    SenzorMQ2();                                                                //Cheama bucla SenzorMQ2    // Se citesc
    SenzorBME280();                                                             //Cheama bucla BME280       // valorile.

    if (BuzzerOn) BuzzerOn = false;                                             // Se inverseazaza o data pe secunda starea variabilei ce determina buzzerul sa sune, 
    else BuzzerOn = true;                                                       //                    ca sa ceeeze un efect sonor de clipire.

  


// HOME //
    if (Menu == 1) {                                                      

      tft.setTextColor(TFT_WHITE, TFT_BLACK);

      if (Temperatura>=30&&BuzzerOn) {   
          digitalWrite(25, HIGH);
          tft.setTextColor(TFT_RED, TFT_BLACK);  
      } 
      else {
        digitalWrite(25, LOW);
      }
       
          
        tft.fillRect(240, 80, 80, 20, TFT_BLACK); // x, y, w, h, color 
        tft.drawFloat(Temperatura, 2, 240, 80, 4);  
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      
        if (Umiditate>=65&&BuzzerOn) {
         digitalWrite(25, HIGH);
        tft.setTextColor(TFT_RED, TFT_BLACK);
      }    
  
        tft.drawFloat(Umiditate, 2, 240, 120, 4);  
        
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.fillRect(240, 160, 100, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(Presiune, 2, 240, 160, 4);   
        tft.fillRect(240, 200, 120, 20, TFT_BLACK); // x, y, w, h, color   
        tft.drawFloat(ConcentratieGaz, 2, 240, 200, 4);
         

    }
// HOME //
        

// MINIM //       
      if(Menu==2){

        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.fillRect(240, 80, 80, 20, TFT_BLACK); // x, y, w, h, color   
        tft.drawFloat(tmin, 2, 240, 80, 4);  
        tft.drawFloat(umin, 2, 240, 120, 4);  
        tft.fillRect(240, 160, 100, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(pmin, 2, 240, 160, 4);  
        tft.fillRect(240, 200, 120, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(VOCmin, 2, 240, 200, 4);  
        
}
// MINIM //


// MAXIME //  
      if(Menu==3){ 

        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        
        tft.fillRect(240, 80, 80, 20, TFT_BLACK); // x, y, w, h, color 
        tft.drawFloat(tmax, 2, 240, 80, 4);   
        tft.drawFloat(umax, 2, 240, 120, 4);    
        tft.fillRect(240, 160, 100, 20, TFT_BLACK); // x, y, w, h, color   
        tft.drawFloat(pmax, 2, 240, 160, 4);       
        tft.fillRect(240, 200, 120, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(VOCmax, 2, 240, 200, 4);  
        
} 
// MAXIME //  


        if(Menu==4){}//build in progress   //meniu setari
        
    }//   SFARSIT Bucla de 1 secunda   // 
}//   SFARSIT VOID LOOP   //
    


  //SENZORI//

  void SenzorBME280() {

    Temperatura = bme.readTemperature();                    //Citire valoarea temperaturii de la BME280
    Presiune = bme.readPressure() / 100.0F;                 //Citire valoarea presiunii atmosferice de la BME280 si transformare din Pa in HPa
    Umiditate = bme.readHumidity();                         //Citire valoarea umiditatii de la BME280


        if(Temperatura>tmax)tmax=Temperatura;               //Variabila tmax ramane cu valoarea celei mai mari temperaturi inregistrate
        if(Temperatura<tmin)tmin=Temperatura;               //Variabila tmin ramane cu valoarea celei mai mici temperaturi inregistrate

        if(Umiditate>umax)umax=Umiditate;                   //Variabila umax ramane cu cea mai mare valoare inregistrata de variabila Umiditate
        if(Umiditate<umin)umin=Umiditate;                   //Variabila umin ramane cu cea mai mica valoare inregistrata de variabila Umiditate

        if(Presiune>pmax)pmax=Presiune;                     //Variabila pmax ramane cu cea mai mare valoare inregistrata de variabila Presiune
        if(Presiune<pmin)pmin=Presiune;                     //Variabila pmin ramane cu cea mai mica valoare inregistrata de variabila Presiune
  }

  void SenzorMQ2() {
    ConcentratieGaz = 0;                                    //Variabila ConcentratieGaz este setata la 0, 

    for (int i=1; i<=5; i++){                               //Bucla for ce se ruleaza de 5 ori, folosita pentru a evita zgomotul din citiri rezultand o valoare stabila
      MQ2.update();                                         //Se actualizeaza valoarea citita de Esp
      ConcentratieGaz += MQ2.readSensor();                  //Variabilei ConcentratieGaz i se aduna 5 valori distincte citite la timpi diferiti 
                            }ConcentratieGaz /= 5;          //Valoarea variabilei ConcentraGaz este impartia la 5 pentru a incheia media aritmetica, returneaza valoarea ce se afiseaza pe ecran

    //MQ2.serialDebug(); // Activeaza trimiterea valorior pe serial pt senzorul MQ2
    //Serial.println(ConcentratieGaz);  //se printeaza pe serial valoarea analogica a pinului MQ2

    if(TimpIncalzireMQ2<millis())                           // Nu se iau in considerare valorile minime si maxime decat dupa ce se incalzeste senzorul
        {      
          if(ConcentratieGaz>VOCmax)VOCmax=ConcentratieGaz; //Variabila VOCmax ramane cu cea mai mare valoare inregistrata de variabila ConcentratieGaz                                        
          if(ConcentratieGaz<VOCmin)VOCmin=ConcentratieGaz; //Variabila VOCmin ramane cu cea mai mica valoare inregistrata de variabila ConcentratieGaz
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
    ActualizareMeniu = true;                                //Actualizare fortata ecran

      if (key[1].justPressed() && Menu != 1) {              //Daca butonul 1 este apasat si nu suntem in meniul 1 ("HOME") se ruleaza↓
        key[1].press(false);                                //Se afiseaza butonul 1 cu culorile textului si a fundalului inversate
        Menu=1;// HOME //                                   //Se seteaza variabila Menu la 1, corespunde meniului 1("HOME")
        Meniu1();                                           //Se afiseaza meniul 1("HOME")
      }


      if (key[2].justPressed() && Menu != 2) {              //Daca butonul 2 este apasat si nu suntem in meniul 2 ("MINIM") se ruleaza↓
        key[2].press(false);                                //Se afiseaza butonul 2 cu culorile textului si a fundalului inversate
        Menu=2;// MINIM //                                  //Se seteaza variabila Menu la 2, corespunde meniului 2("MINIM")
        Meniu2();                                           //Se afiseaza meniul 2("MINIM")
      }


      if (key[3].justPressed() && Menu != 3) {              //Daca butonul 3 este apasat si nu suntem in meniul 3 ("MAXIM") se ruleaza↓
        key[3].press(false);                                //Se afiseaza butonul 3 cu culorile textului si a fundalului inversate
        Menu=3;// MAXIM //                                  //Se seteaza variabila Menu la 3, corespunde meniului 3("MAXIM")
        Meniu3();                                           //Se afiseaza meniul 3("MAXIM")
      }


      if (key[4].justPressed() && Menu != 4) {              //Daca butonul 4 este apasat si nu suntem in meniul 4 ("SETARI") se ruleaza↓
        key[4].press(false);                                //Se afiseaza butonul 4 cu culorile textului si a fundalului inversate
        Menu=4;// SETARI //                                 //Se seteaza variabila Menu la 4, corespunde meniului 4("SETARI")
        Meniu4();                                           //Se afiseaza meniul 4("SETARI")
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

  void Meniu1() {
    tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
    key[1].drawButton(true);                           //Afiseaza butonul"Home"
    key[2].drawButton();                               //Afiseaza butonul"Minim"
    key[3].drawButton();                               //Afiseaza butonul"Maxim"
    key[4].drawButton();                               //Afiseaza butonul"Setari"

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.drawString("Temperatura", 10, 80, 4);
    tft.drawString("*C", 360, 80, 4);
    tft.drawString("Umiditate", 10, 120, 4);
    tft.drawString("%", 360, 120, 4); 
    tft.drawString("Presiune", 10, 160, 4); 
    tft.drawString("hPa", 360, 160, 4);
    tft.drawString("VOC", 10, 200, 4);
    tft.drawString("PPM", 360, 200, 4);
  }

  void Meniu2() {
    tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
    key[1].drawButton();                               //Afiseaza butonul"Home"
    key[2].drawButton(true);                           //Afiseaza butonul"Minim"
    key[3].drawButton();                               //Afiseaza butonul"Maxim"
    key[4].drawButton();                               //Afiseaza butonul"Setari"

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.drawString("Temperatura Min.", 10, 80, 4);
    tft.drawString("*C", 360, 80, 4);
    tft.drawString("Umiditate Min.", 10, 120, 4); 
    tft.drawString("%", 360, 120, 4);
    tft.drawString("Presiune Min.", 10, 160, 4); 
    tft.drawString("hPa", 360, 160, 4); 
    tft.drawString("VOC Min.", 10, 200, 4);
    tft.drawString("PPM", 360, 200, 4); 
 
  }

  void Meniu3() {
    tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
    key[1].drawButton();                               //Afiseaza butonul"Home"
    key[2].drawButton();                               //Afiseaza butonul"Minim"
    key[3].drawButton(true);                           //Afiseaza butonul"Maxim"
    key[4].drawButton();                               //Afiseaza butonul"Setari"

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.drawString("Temperatura Max.", 10, 80, 4);
    tft.drawString("*C", 360, 80, 4);
    tft.drawString("Umiditate Max.", 10, 120, 4);
    tft.drawString("%", 360, 120, 4);
    tft.drawString("Presiune Max.", 10, 160, 4);
    tft.drawString("hPa", 360, 160, 4);
    tft.drawString("VOC Max.", 10, 200, 4);
    tft.drawString("PPM", 360, 200, 4);
  }

  void Meniu4() {

    tft.fillScreen(TFT_BLACK);                         //Umplere ecran cu negru
    key[1].drawButton();                               //Afiseaza butonul"Home"
    key[2].drawButton();                               //Afiseaza butonul"Minim"
    key[3].drawButton();                               //Afiseaza butonul"Maxim"
    key[4].drawButton(true);                           //Afiseaza butonul"Setari"

    






  } 