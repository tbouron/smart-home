#ifndef DotFont_h
#define DotFont_h
#define COLORED 0
#define UNCOLORED 1

#include <Arduino.h>
#include <epdpaint.h>

class DotFont {
  public:
    DotFont(const Paint* paint);
    void DrawCharAt(int x, int y, char ascii_char[], int radius);
  private:
    const Paint* _paint;
    static const byte DOT_FONT[10][5] PROGMEM;
    void _DrawCircleCharAt(int x, int y, unsigned char c[5], int radius);
};

#endif
