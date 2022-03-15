#include <Wire.h>
#include "FT62XXTouchScreen.h"
#include "Free_Fonts.h"
#include <SPI.h>
#include <TFT_eSPI.h>  // Hardware-specific library
int buzzer = 25;
bool BuzzerOn;

//BME280//
#include "Adafruit_Sensor.h"
#include "Adafruit_BME280.h"
Adafruit_BME280 bme;  //foloseste I2C
float Temperatura, Umiditate, Presiune;
float tmax, umax, pmax;
float tmin = 100, umin = 100, pmin = 1500;

//Senzor MQ2 - Metan//

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
int TimpIncalzireMQ2 = 60000;// Timp de asteptare pana se incalzeste senzorul de gaze volatile

/**********Globals****************/
MQUnifiedsensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);
/**********Globals****************/


//Includem fisierul cu imagini
#include "Imagini.h"  // convertor imagine to hex  http://rinkydinkelectronics.com/t_imageconverter565.php

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

//ButoaneFizice//
int ButonSus = 17;
int ButonJos = 2;

int TFT_BL = 23;  // Backlight

#define DISPLAY_WIDTH 480   //Latimea Ecranului
#define DISPLAY_HEIGHT 320  //Inaltimea ecranului

int XMiddle = DISPLAY_WIDTH / 2;   //coordonata pe X din mijlocul ecranului
int YMiddle = DISPLAY_HEIGHT / 2;  //coordonata pe Y din mijlocul ecranului

FT62XXTouchScreen touchScreen = FT62XXTouchScreen(DISPLAY_HEIGHT, 18, 19);  //(DISPLAY_HEIGHT, PIN_SDA, PIN_SCL);

int16_t h;
int16_t w;

bool ActualizareMeniu;//Folosita pentru actualizarea imediata a ecranului
int Menu = 1;

//Variabile pt refresh valori ecran/
unsigned long TimpAnterior = 0;
unsigned long TimpActual = 0;
unsigned int TimpRefresh = 1000;  // 1 secunda


// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[16];

int TextSize = 1;

/***/

void setup() {

  pinMode(ButonSus, INPUT_PULLUP);
  pinMode(ButonJos, INPUT_PULLUP);
  pinMode(TFT_BL, OUTPUT);  // Backlight
  digitalWrite(TFT_BL, 128);
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200);

  tft.init();
  touchScreen.begin();
  tft.setTextSize(TextSize);
  tft.setFreeFont(FF6);

  h = tft.height();
  w = tft.width();

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  //tft.setFreeFont(FS18);

  //key[5].initButton(&tft, 45, 295,  70, 20, TFT_RED, TFT_BLACK, TFT_ORANGE, "<<", 1);// x, y, w, h, culoare rama,culoare buton, culoare text.

  // Newer function instead accepts upper-left corner & size
  //key[1].initButtonUL(&tft,, int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize)
  key[1].initButtonUL(&tft, 4, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Home", TextSize);  // x, y, w, h, culoare rama,culoare buton, culoare text, marime text.
  key[1].drawButton();

  key[2].initButtonUL(&tft, 123, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Minim", TextSize);  // x, y, w, h, culoare rama,culoare buton, culoare text, marime text.
  key[2].drawButton();

  key[3].initButtonUL(&tft, 242, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Maxim", TextSize);  // x, y, w, h, culoare rama,culoare buton, culoare text.
  key[3].drawButton(); 

  key[4].initButtonUL(&tft, 361, 255, 115, 60, TFT_WHITE, TFT_WHITE, TFT_BLACK, "Setari", TextSize);  // x, y, w, h, culoare rama,culoare buton, culoare text, marime text.
  key[4].drawButton();    
  //tft.pushImage(280, 275, 40, 40, CogWheel);                                                    //Afisare iconita Setari

  //BME280//

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
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


    // / Check if any key coordinate boxes contain the touch coordinates
    for (uint8_t b = 0; b < 15; b++) {
      // if (pressed && key[b].contains(touchPos.xPos, touchPos.yPos)) {
      if (key[b].contains(touchPos.xPos, touchPos.yPos)) {
        key[b].press(true);  // tell the button it is pressed
      } else {
        key[b].press(false);  // tell the button it is NOT pressed
      }
    }
    
    Butoane();  //decide ce buton afisat este apasat
  }
  ButoaneFizice();  //verifica apasarea butoanelor fizice

  TimpActual = millis();
  if (TimpActual - TimpAnterior >= TimpRefresh || ActualizareMeniu) {//Bucla se ruleaza o data pe secunda
    TimpAnterior = millis();

    //SenzorMQ2();     //Cheama bucla SenzorMQ2
    //SenzorBME280();  //Cheama bucla BME280
    if (BuzzerOn) BuzzerOn = false;
    else BuzzerOn = true;
    

    if (Menu == 1) {// HOME //

      tft.setTextColor(TFT_WHITE, TFT_BLACK);

      if (Temperatura>=30&&BuzzerOn) {   
          digitalWrite(25, HIGH);
          tft.setTextColor(TFT_RED, TFT_BLACK);  
      } 
      else {
        digitalWrite(25, LOW);
      }
       
        tft.drawString("Temperatura", 10, 80, 4);  
        tft.fillRect(240, 80, 80, 20, TFT_BLACK); // x, y, w, h, color 
        tft.drawFloat(Temperatura, 2, 240, 80, 4);  
        tft.drawString("*C", 360, 80, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
      
        if (Umiditate>=65&&BuzzerOn) {
         digitalWrite(25, HIGH);
        tft.setTextColor(TFT_RED, TFT_BLACK);
      }    

        tft.drawString("Umiditate", 10, 120, 4);   
        tft.drawFloat(Umiditate, 2, 240, 120, 4);  
        tft.drawString("%", 360, 120, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);


        tft.drawString("Presiune", 10, 160, 4); 
        tft.fillRect(240, 160, 100, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(Presiune, 2, 240, 160, 4);  
        tft.drawString("hPa", 360, 160, 4);

        tft.drawString("VOC", 10, 200, 4);
        tft.fillRect(240, 200, 120, 20, TFT_BLACK); // x, y, w, h, color   
        tft.drawFloat(ConcentratieGaz, 2, 240, 200, 4);
        tft.drawString("PPM", 360, 200, 4); 

    }

        

        
      if(Menu==2){// MINIME //

        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.drawString("Temperatura Min.", 10, 80, 4); 
        tft.fillRect(240, 80, 80, 20, TFT_BLACK); // x, y, w, h, color   
        tft.drawFloat(tmin, 2, 240, 80, 4);  
        tft.drawString("*C", 360, 80, 4); 

        tft.drawString("Umiditate Min.", 10, 120, 4);   
        tft.drawFloat(umin, 2, 240, 120, 4);  
        tft.drawString("%", 360, 120, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.drawString("Presiune Min.", 10, 160, 4);  
        tft.fillRect(240, 160, 100, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(pmin, 2, 240, 160, 4);  
        tft.drawString("hPa", 360, 160, 4);
 
        tft.drawString("VOC Min.", 10, 200, 4);  
        tft.fillRect(240, 200, 120, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(VOCmin, 2, 240, 200, 4);  
        tft.drawString("PPM", 360, 200, 4);
}

      if(Menu==3){// MAXIME //   

        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.drawString("Temperatura Max.", 10, 80, 4);   
        tft.fillRect(240, 80, 80, 20, TFT_BLACK); // x, y, w, h, color 
        tft.drawFloat(tmax, 2, 240, 80, 4);  
        tft.drawString("*C", 360, 80, 4); 
                
        tft.drawString("Umiditate Max.", 10, 120, 4);   
        tft.drawFloat(umax, 2, 240, 120, 4);  
        tft.drawString("%", 360, 120, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.drawString("Presiune Max.", 10, 160, 4); 
        tft.fillRect(240, 160, 100, 20, TFT_BLACK); // x, y, w, h, color   
        tft.drawFloat(pmax, 2, 240, 160, 4);  
        tft.drawString("hPa", 360, 160, 4);
        
        tft.drawString("VOC Max.", 10, 200, 4);  
        tft.fillRect(240, 200, 120, 20, TFT_BLACK); // x, y, w, h, color  
        tft.drawFloat(VOCmax, 2, 240, 200, 4);  
        tft.drawString("PPM", 360, 200, 4);
  } 

        if(Menu==4){//build in progress(drum inchis)
        }

      SenzorMQ2();     //Cheama bucla SenzorMQ2
      SenzorBME280();  //Cheama bucla BME280 
    }
    delay(20);  

          
    
       
}//***SFARSIT VOID LOOP***//
    


  //SENZORI//

  void SenzorBME280() {

    Temperatura = bme.readTemperature();
    Serial.print("Temperature = ");
    Serial.print(Temperatura);
    Serial.println(" *C");

    Presiune = bme.readPressure() / 100.0F;  //transformare din HPa in Pa
    Serial.print("Pressure = ");
    Serial.print(Presiune);
    Serial.println(" hPa");

    Umiditate = bme.readHumidity();
    Serial.print("Humidity = ");
    Serial.print(Umiditate);
    Serial.println(" %");

        if(tmax<Temperatura)tmax=Temperatura;// 
        if(Temperatura<tmin)tmin=Temperatura;//
        if(umax<Umiditate)umax=Umiditate;//
        if(Umiditate<umin)umin=Umiditate;//
        if(pmax<Presiune)pmax=Presiune;//  
        if(Presiune<pmin)pmin=Presiune;//
  }

  void SenzorMQ2() {

  

    ConcentratieGaz = 0;
    for (int i=1; i<=10; i++){
      MQ2.update(); // Update data, the arduino will be read the voltage on the analog pin
      ConcentratieGaz += MQ2.readSensor(); // Sensor will read PPM concentration using the model and a and b values setted before or in the setup

      
}  
 ConcentratieGaz /= 10;

    //**********MQ2.serialDebug(); // Activeaza trimiterea valorior pe serial pt senzorul MQ2
    //Serial.println(ConcentratieGaz);  //se printeaza pe serial valoarea analogica a pinului MQ2
    //SA SE CALCULEZE VALOAREA PARTICULELOR DUPA FORMULA DE MAI JOS

    // How to Detect Concentration of Gas by Using MQ2 Sensor
    //https://www.instructables.com/How-to-Detect-Concentration-of-Gas-by-Using-MQ2-Se/


    //COMPARARE NIVEL DEPASIT
    if(TimpIncalzireMQ2<millis())
        {                                              // Nu luam valorile minime si maxime decat dupa ce se incalzeste senzorul
          if(ConcentratieGaz<VOCmin)VOCmin=ConcentratieGaz;
          if(VOCmax<ConcentratieGaz)VOCmax=ConcentratieGaz;
        }
  }


  //BUTOANE//

  void ButoaneFizice() {
    bool ButonSusApasat = !digitalRead(ButonSus);
    bool ButonJosApasat = !digitalRead(ButonJos);

    if (ButonJosApasat) {
      Menu--;
      if (Menu < 1) Menu = 4;
      Serial.println(Menu);
      ActualizareMeniu = true;//Actualizare fortata meniu
      delay(300);  //Button debouncing
      Meniuri();
    }


    if (ButonSusApasat) {
      Menu++;
      if (Menu > 4) Menu = 1;
      Serial.println(Menu);
      ActualizareMeniu = true;//Actualizare fortata meniu
      delay(300);  // Button debouncing
      Meniuri();
    }
  }

  void Butoane() {
    ActualizareMeniu = true;//Actualizare fortata meniu

    // else {
    //  for (uint8_t b = 0; b < 15; b++) {
    //   key[b].press(false);  // tell the button it is NOT pressed
    // }
    //}

      if (key[1].justPressed()) {
        // HOME //

        key[1].press(false);
        Menu=1;
        

        //Serial.println(Menu);
        delay(200);  // UI debouncing
        Meniu1();
      }



      if (key[2].justPressed()) {
        // MINIM //

        key[2].press(false);
        Menu=2;
        

        //Serial.println(Menu);
        delay(200);  // UI debouncing
        Meniu2();
      }

      if (key[3].justPressed()) {
        // MAXIM //

        key[3].press(false);
        Menu=3;
        //Serial.println(Menu);
        delay(200);  // UI debouncing
        Meniu3();
      }

      if (key[4].justPressed()) {
        // SETARI //
        Menu=4;
        key[4].press(false);
        Meniu4();
      }
  }


  //MENIURI//
  void Meniuri() {
    tft.fillScreen(TFT_BLACK);
    if (Menu == 1) Meniu1();
    else if (Menu == 2)
      Meniu2();
    else if (Menu == 3)
      Meniu3();
    else if (Menu == 4)
      Meniu4();
      
  }

  void Meniu1() {
    tft.fillScreen(TFT_BLACK);
    key[1].drawButton(true);                               //Afiseaza butonul"Home"
    key[2].drawButton();                               //Afiseaza butonul"Minim"
    key[3].drawButton();                               //Afiseaza butonul"Maxim"
    key[4].drawButton();                               //Afiseaza butonul"Setari"
    tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE);  //x, y, w, h, r, color
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("Meniu1", 15, 20, 4);
  }

  void Meniu2() {
    tft.fillScreen(TFT_BLACK);
    key[1].drawButton();                               //Afiseaza butonul"Home"
    key[2].drawButton(true);                               //Afiseaza butonul"Minim"
    key[3].drawButton();                               //Afiseaza butonul"Maxim"
    key[4].drawButton();                               //Afiseaza butonul"Setari"
    tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE);  //x, y, w, h, r, color
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("Minime", 15, 20, 4);
  }

  void Meniu3() {
    tft.fillScreen(TFT_BLACK);
    key[1].drawButton();                               //Afiseaza butonul"Home"
    key[2].drawButton();                               //Afiseaza butonul"Minim"
    key[3].drawButton(true);                               //Afiseaza butonul"Maxim"
    key[4].drawButton();                               //Afiseaza butonul"Setari"
    tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE);  //x, y, w, h, r, color
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("Maxime", 15, 20, 4);
  }

  void Meniu4() {

    tft.fillScreen(TFT_BLACK);
    key[1].drawButton();                               //Afiseaza butonul"Home"
    key[2].drawButton();                               //Afiseaza butonul"Minim"
    key[3].drawButton();                           //Afiseaza butonul"Maxim"
    key[4].drawButton(true);                               //Afiseaza butonul"Setari"tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE);  //x, y, w, h, r, color
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString("Setari", 15, 20, 4);
  }