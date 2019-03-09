#include <Arduino.h>
#include <avr/pgmspace.h>
#include <epdpaint.h>
#include "DotFont.h"

const byte DotFont::DOT_FONT[10][5] PROGMEM {
  {
    // @48 '0' (5 pixels wide)
    0x7C, //  ###
    0x82, // #   #
    0x82, // #   #
    0x82, // #   #
    0x7C  //  ### 
  },
  {
    // @49 '1' (5 pixels wide)
    0x00, //    ##
    0x00, //     #
    0x00, //     #
    0x02, //     #
    0xFE  //     #  
  },
  {
    // @50 '2' (5 pixels wide)
    0xE4, //  ###
    0x92, //     #
    0x92, //  ###
    0x92, // #   
    0x4C  //  ###
  },
  {
    // @51 '3' (5 pixels wide)
    0x44, //  ###
    0x92, //     #
    0x92, //  ###
    0x92, //     #
    0x6C  //  ###
  },
  {
    // @52 '4' (5 pixels wide)
    0x1E, // #   #
    0x10, // #   #
    0x10, // #####
    0x10, //     #
    0xFE  //     #
  },
  {
    // @53 '5' (5 pixels wide)
    0x5E, //  ###
    0x92, // #
    0x92, //  ###
    0x92, //     #
    0x62  //  ### 
  },
  {
    // @54 '6' (5 pixels wide)
    0x7C, //  ###
    0x92, // #
    0x92, // ####
    0x92, // #   #
    0x64  //  ###
  },
  {
    // @55 '7' (5 pixels wide)
    0x02, //  ###
    0xE2, //     #
    0x12, //     #
    0x0A, //     #
    0x06  //     #
  },
  {
    
    // @56 '8' (5 pixels wide)
    0x6C, //  ###
    0x92, // #   #
    0x92, //  ###
    0x92, // #   #
    0x6C  //  ###
  },
  {
    // @57 '9' (5 pixels wide)
    0x4C, //  ###
    0x92, // #   #
    0x92, //  ###
    0x92, //     #
    0x7C  //  ###
  }
};

DotFont::DotFont(const Paint* paint) {
  this->_paint = paint;
}

void DotFont::DrawCharAt(int x, int y, char ascii_char[], int radius) {
  int left = x;
  for (int i = 0; i < strlen(ascii_char); i++) {
    if (ascii_char[i] >= 48 and ascii_char[i] <= 57) {
      char digit[5];
      memcpy_P(digit, DOT_FONT[ascii_char[i]-48], sizeof digit);
      this->_DrawCircleCharAt(left, y, digit, radius);
      if (ascii_char[i+1] == 49) {
        left += radius*10;
      } else {
        left += radius*20;
      }
    } else if (ascii_char[i] == 58) {
       this->_paint->DrawFilledCircle(left, y+radius*15, radius, COLORED);
       this->_paint->DrawFilledCircle(left, y+radius*4, radius, COLORED);
       left+=radius*8;
    } else if (ascii_char[i] == 46) {
       this->_paint->DrawFilledCircle(left, y+radius*18, radius, COLORED);
       left += radius*8;
    } else if (ascii_char[i] == 45) {
       this->_paint->DrawFilledCircle(left+radius*9, y+radius*8, radius, COLORED);
       this->_paint->DrawFilledCircle(left+radius*12, y+radius*8, radius, COLORED);
       left += radius*20;
    } else {
       left += radius*20;
    }  
  }
}

void DotFont::_DrawCircleCharAt(int xbegin, int ybegin, unsigned char c[5], int radius) {
  char ctemp;
  for (int x = 0; x < 5; x++) {
    ctemp = c[x];
    for (int y = 0; y < 7; y++) {
      ctemp >>= 1;
      if (ctemp & 01) {
        this->_paint->DrawFilledCircle(x*radius*3+xbegin, y*radius*3+ybegin, radius, COLORED);
      }
    }
  }
}
