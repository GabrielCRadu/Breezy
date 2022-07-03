/*



*/
#include <Wire.h>
#include "FT62XXTouchScreen.h"



//#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320
//#define BLACK 0x0000
//#define WHITE 0xFFFF

#include <SPI.h>

#include <TFT_eSPI.h>  // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

FT62XXTouchScreen touchScreen = FT62XXTouchScreen(DISPLAY_HEIGHT, 18, 19);  //(DISPLAY_HEIGHT, PIN_SDA, PIN_SCL);

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


  //key[5].initButton(&tft, 45, 295,  70, 20, TFT_RED, TFT_BLACK, TFT_ORANGE, "<<", 1);// x, y, w, h, culoare rama,culoare buton, culoare text.

  // Newer function instead accepts upper-left corner & size
  //key[1].initButtonUL(&tft,, int16_t x1, int16_t y1, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, uint16_t textcolor, char *label, uint8_t textsize)
  key[1].initButtonUL(&tft, 0, 0, 120, 40, TFT_RED, TFT_BLACK, TFT_ORANGE, "<<", 1);  // x, y, w, h, culoare rama,culoare buton, culoare text.
  key[1].drawButton();
}

/***********************************************************************************************************************************/
void loop() {

  TouchPoint touchPos = touchScreen.read();

  if (touchPos.touched) {
    Serial.printf("X: %d, Y: %d\n", touchPos.xPos, touchPos.yPos);
    tft.drawCircle(touchPos.xPos, touchPos.yPos, 10, TFT_SILVER);

    // / Check if any key coordinate boxes contain the touch coordinates
    for (uint8_t b = 0; b < 15; b++) {
      // if (pressed && key[b].contains(touchPos.xPos, touchPos.yPos)) {
      if (key[b].contains(touchPos.xPos, touchPos.yPos)) {
        key[b].press(true);  // tell the button it is pressed
      } else {
        key[b].press(false);  // tell the button it is NOT pressed
      }
    }
  } else {
    for (uint8_t b = 0; b < 15; b++) {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }


  // Check if any key has changed state
  for (uint8_t b = 0; b < 15; b++) {



    if (key[b].justReleased()) key[b].drawButton();  // draw normal

    if (key[b].justPressed()) {
      key[b].drawButton(true);  // draw invert
      Serial.printf("justPressed: %d\n", b);
     // Serial.println(b);


      delay(10);  // UI debouncing
    }
  }



  delay(20);  // Delay to reduce loop rate (reduces flicker caused by aliasing with TFT screen refresh rate)
}
