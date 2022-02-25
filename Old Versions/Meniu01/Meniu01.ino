#include <Wire.h>
#include "FT62XXTouchScreen.h"
#include "Free_Fonts.h"


//#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320


#include <SPI.h>

#include <TFT_eSPI.h>  // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

FT62XXTouchScreen touchScreen = FT62XXTouchScreen(DISPLAY_HEIGHT, 18, 19);  //(DISPLAY_HEIGHT, PIN_SDA, PIN_SCL);

int Menu;
int16_t h;
int16_t w;


int TFT_BL = 23;  // Backlight

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[16];

/***********************************************************************************************************************************/

void setup() {

  pinMode(TFT_BL, OUTPUT);  // Backlight
  digitalWrite(TFT_BL, 128);

  Serial.begin(115200);

  tft.init();
  touchScreen.begin();

  h = tft.height();
  w = tft.width();

  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(FF33);

  //key[5].initButton(&tft, 45, 295,  70, 20, TFT_RED, TFT_BLACK, TFT_ORANGE, "<<", 1);// x, y, w, h, culoare rama,culoare buton, culoare text.

  // Newer function instead accepts upper-left corner & size
  //key[1].initButtonUL(&tft,, int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize)
  key[1].initButtonUL(&tft, 0, 100, 40, 120, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLACK, "<", 2);  // x, y, w, h, culoare rama,culoare buton, culoare text.
  key[1].drawButton();

  key[2].initButtonUL(&tft, 440, 100, 40, 120, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLACK, ">", 2);  // x, y, w, h, culoare rama,culoare buton, culoare text.
  key[2].drawButton();
}

/**************************************************VOID LOOP************************************************************************/
void loop() {
  TouchPoint touchPos = touchScreen.read();

  if (touchPos.touched) {
    VerificareButoane(); 
  }






  //delay(20);  // Delay to reduce loop rate (reduces flicker caused by aliasing with TFT screen refresh rate)
}
/*******BUTOANE*******/
void VerificareButoane(){ 

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
  // else {
  //  for (uint8_t b = 0; b < 15; b++) {
   //   key[b].press(false);  // tell the button it is NOT pressed
   // }   
  //}
  
  if (key[1].justPressed()) {
      key[1].drawButton(true);  // draw invert
      Menu--;
      Serial.println(Menu);
      delay(100);  // UI debouncing
    }



    if (key[2].justPressed()) {
      key[2].drawButton(true);  // draw invert
      Menu++;
      Serial.println(Menu);
      delay(100);  // UI debouncing
    }

  // Check if any key has changed state
  //for (uint8_t b = 0; b < 15; b++) {

   // if (key[b].justPressed()) {
   //   key[b].drawButton(true);  // draw invert
      
   //   delay(100);  // UI debouncing
   // }



   // if (key[b].justReleased()) key[b].drawButton();  // draw normal




 // }






    }


/*******MENIURI*******/

void Meniu1() {
  tft.fillScreen(TFT_BLACK);

}

void Meniu2() {
  tft.fillScreen(TFT_BLACK);

}