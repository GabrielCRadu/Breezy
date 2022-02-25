#include <Wire.h>
#include "FT62XXTouchScreen.h"
#include "Free_Fonts.h"
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

//Includem fisierul cu imagini
#include "Imagini.h"  // convertor imagine to hex  http://rinkydinkelectronics.com/t_imageconverter565.php

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

#define DISPLAY_WIDTH  480 //Latimea Ecranului
#define DISPLAY_HEIGHT 320 //Inaltimea ecranului

int XMiddle = DISPLAY_WIDTH / 2; //coordonata pe X din mijlocul ecranului
int YMiddle = DISPLAY_HEIGHT / 2; //coordonata pe Y din mijlocul ecranului

FT62XXTouchScreen touchScreen = FT62XXTouchScreen(DISPLAY_HEIGHT, 18, 19);  //(DISPLAY_HEIGHT, PIN_SDA, PIN_SCL);

int Menu = 1;
int16_t h;
int16_t w;

bool MeniuPrincipal;


//*****************************************//
//**********Senzor MQ2 - Metan*************//
int MQ2 = 34; //pinul la care este legat senzorul
int ValoareMQ2 = 0;

int TFT_BL = 23;  // Backlight

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[16];

/***********************************************************************************************************************************/

void setup() {
  pinMode(MQ2, INPUT); //definim pinul la care este legat senzorul MQ2 ca intrare
  pinMode(TFT_BL, OUTPUT);  // Backlight
  digitalWrite(TFT_BL, 128);


  Serial.begin(115200);

  tft.init();
  touchScreen.begin();

  h = tft.height();
  w = tft.width();

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FS18);

  //key[5].initButton(&tft, 45, 295,  70, 20, TFT_RED, TFT_BLACK, TFT_ORANGE, "<<", 1);// x, y, w, h, culoare rama,culoare buton, culoare text.

  // Newer function instead accepts upper-left corner & size
  //key[1].initButtonUL(&tft,, int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize)
  key[1].initButtonUL(&tft, 10, 230, 60, 80, TFT_WHITE, TFT_WHITE, TFT_BLACK, "<", 1);  // x, y, w, h, culoare rama,culoare buton, culoare text.
  key[1].drawButton();

  key[2].initButtonUL(&tft, 410, 230, 60, 80, TFT_WHITE, TFT_WHITE, TFT_BLACK, ">", 1);  // x, y, w, h, culoare rama,culoare buton, culoare text.
  key[2].drawButton();

  key[3].initButtonUL(&tft, 430, 10, 40, 40, TFT_DARKGREY, TFT_LIGHTGREY, TFT_BLACK, "Setari", 1);  // x, y, w, h, culoare rama,culoare buton, culoare text.
  tft.pushImage(430, 10, 40, 40, CogWheel);   //Afisare iconita Setari


  MeniuPrincipal = true;
  Meniu1();
}

/**************************************************VOID LOOP************************************************************************/
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
    Butoane();
  }
  SenzorMQ2();//Cheama bucla SenzorMQ2
  delay(20);  // Delay to reduce loop rate (reduces flicker caused by aliasing with TFT screen refresh rate)
}



/*******SENZORI*******/
void SenzorMQ2() {
  ValoareMQ2 = analogRead(MQ2);
  
  Serial.println(ValoareMQ2);//se printeaza pe serial valoarea analogica a pinului MQ2
  delay(500);

  //SA SE CALCULEZE PPM
}


/*******BUTOANE*******/
void Butoane() {



  // else {
  //  for (uint8_t b = 0; b < 15; b++) {
  //   key[b].press(false);  // tell the button it is NOT pressed
  // }
  //}

  if (MeniuPrincipal == true) {  //-MENIURI PRINCIPALE
    tft.pushImage(430, 10, 40, 40, CogWheel);   //Afisare iconita Setari
    if (key[1].justPressed()) {/*<*/

      key[1].press(false);
      Menu--;
      if (Menu < 1)Menu = 3;

      Serial.println(Menu);
      delay(200);  // UI debouncing
      MenuHandler();

    }



    if (key[2].justPressed()) {/*>*/

      key[2].press(false);
      Menu++;
      if (Menu > 3)Menu = 1;

      Serial.println(Menu);
      delay(200);  // UI debouncing
      MenuHandler();

    }

    if (key[3].justPressed()) {/*SETARI*/

      key[3].press(false);
      Setari();
      MeniuPrincipal = false;

    }
  }//-MENIURI PRINCIPALE


  if (MeniuPrincipal == false) { //-MENIURI SECUNDARE

    if (key[1].justPressed()) {/*Iesire*/
      key[1].press(false);
      MeniuPrincipal = true;
      tft.fillScreen(TFT_BLACK);
      tft.pushImage(430, 10, 40, 40, CogWheel);   //Afisare iconita Setari
      MenuHandler();

    }
  }








  // Check if any key has changed state
  //for (uint8_t b = 0; b < 15; b++) {

  // if (key[b].justPressed()) {
  //   key[b].drawButton(true);  // draw invert

  //   delay(100);  // UI debouncing
  // }


  //if (key[1].justReleased())key[1].press(false);  // tell the button it is NOT pressed
  // if (key[b].justReleased()) key[b].drawButton();  // draw normal




  // }
}


/*******MENIURI*******/
void MenuHandler() {
  if (Menu == 1) Meniu1();
  else if (Menu == 2) Meniu2();
  else if (Menu == 3) Meniu3();

}



void Meniu1() {
  //tft.setTextSize(1);
  //tft.fillScreen(TFT_BLACK);
  key[1].drawButton();//Afiseaza butonul"<"
  key[2].drawButton();//Afiseaza butonul">"
  tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE); //x, y, w, h, r, color
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Meniu1", 15, 20, 4);

}

void Meniu2() {
  //tft.setTextSize(1);
  //tft.fillScreen(TFT_BLACK);
  key[1].drawButton();//Afiseaza butonul"<"
  key[2].drawButton();//Afiseaza butonul">"
  tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE); //x, y, w, h, r, color
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Meniu2", 15, 20, 4);
}

void Meniu3() {
  //tft.setTextSize(1);
  //tft.fillScreen(TFT_BLACK);
  key[1].drawButton();//Afiseaza butonul"<"
  key[2].drawButton();//Afiseaza butonul">"
  tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE); //x, y, w, h, r, color
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Meniu3", 15, 20, 4);
}

void Setari() {

  tft.fillScreen(TFT_BLACK);
  key[1].drawButton();//Afiseaza butonul Inapoi (Folosesc Butonul "Stanga" pentru a iesi din meniu)
  tft.fillRoundRect(5, 10, 100, 40, 10, TFT_WHITE); //x, y, w, h, r, color
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawString("Setari", 15, 20, 4);
}
