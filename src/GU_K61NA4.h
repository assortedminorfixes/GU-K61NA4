#ifndef __GU_K61NA4__
#define __GU_K61NA4__
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#ifdef USE_NATIVE

// Defined fonts which exactly mimic the fonts on the VFD but may not be published
#include "NoritakeFonts.h"

#else 
// Use fonts from https://github.com/robjen/GFX_fonts
// Not quite right, but open source.
#include <Font5x5Fixed.h>
#define Noritake5w Font5x5Fixed

#include <Font5x7FixedMono.h>
#define Noritake5x7 Font5x7FixedMono

#include <FreeMono12pt7b.h>
#define Noritake10x14 FreeMono12pt7b

#endif // USE_NATIVE

typedef enum e_GU_NativeFont_IDX {
    fNoFont = 0x1B,
    fNMini = 0x1C,
    fN5x7 = 0x1D,
    fN10x14 = 0x1E
} GU_NativeFont_IDX;

class GU_K61NA4: public GFXcanvas1 {
public:
    GU_K61NA4(uint8_t sbusy, uint8_t reset, uint8_t cs, uint8_t din, uint8_t dout, uint8_t clk, uint8_t width, uint8_t height);
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillScreen(uint16_t color);
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg);
    void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
    void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
    // Optional and probably not necessary to change
    // void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    void setTextSize(uint8_t s_x, uint8_t s_y);

    void getCharRange(uint8_t *first, uint8_t *last);

    size_t write(uint8_t c);

    void display();
    void displayOn(bool on);
    void setBrightness(uint8_t level);
    void clear();

    void setFont(const GFXfont *f);

    void setDelayMultiplier(float mult);

    void setRotation(uint8_t r);

    void printCentered(const char* str);
    void printCentered(String str);

    void printCentered(const char* str, int16_t x, int16_t y, uint16_t w, uint16_t h);
    void printCentered(String str, int16_t x, int16_t y, uint16_t w, uint16_t h);

    void setCursorFlash(bool flash);
    void toggleCursorFlash();

    void debug_printf(const char * str, ...);

    //void write(uint8_t c);
    //size_t write(uint8_t c);

    int printf_r(const char * str, ...);

private:
    uint8_t sin;
    uint8_t sbusy;
    uint8_t sck;
    uint8_t reset;
    uint8_t cs;
    uint8_t din;
    uint8_t dout;
    uint8_t clk;

    const int init_pos[2] = {0, 5};
    const GFXfont* init_font = &Noritake5x7;

    bool initialized = false;

    void vfdSetCursor(uint8_t x, uint8_t y);
    //void writeByte(int8_t b);
    //void vfdWriteImageData(uint8_t * data, uint8_t pixelWidth, uint8_t byteHeight);

    void expandBounds(int16_t x, int16_t y, int16_t w, int16_t h);
    void expandBounds(int16_t x, int16_t y);
    void resetBounds();

    void vfdSetArea(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb);
    void vfdClearArea(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb);
    void vfdInvertArea(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb);
    void vfdSetOutline(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb);
    void vfdClearOutline(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb);

    void vfdAreaCommand(uint8_t command, uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb);
    void vfdGraphicWrite(uint8_t len, uint8_t * bytes);

    void endWrite();

    // May not be supported (documentation mentions only available on software version 3)
    //void vfdGraphicAreaWrite(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb, uint8_t len, uint8_t * bytes);

    bool horizontal_gfx;
    bool horizontal_cursor;
    bool cursor_reverse;
    bool underscore_cursor;
    bool underscore_flash;
    uint8_t writetype;

    void vfdSetWriteMode(bool horizontal_gfx, bool horizontal_cursor, bool cursor_reverse,
                                bool underscore_cursor, bool underscore_flash, uint8_t writetype);

    void vfdSetWriteMode(bool horizontal, bool underscore_cursor,
                                bool underscore_flash, uint8_t writetype);

    void vfdSetWriteMode();

    void vfdToggleHexMode();

    void vfdSelectFont(GU_NativeFont_IDX fontcode);

    void vfdWriteCharacter(char c);

    
    void VFDsetPixel();
    void VFDclearPixel();
    void commanddelay(uint16_t delay_us);
    void commanddelay() {
        commanddelay(50);
    };
    void waitdelay();

    void syncPostInit();

    void vfdInvalidateCursor();

    int16_t minX;
    int16_t maxX;
    int16_t minY;
    int16_t maxY;
    uint32_t lastmillis;
    uint32_t lastmicros;
    uint32_t nextmillis;
    uint32_t nextmicros;

    int16_t vfdCursor_x;
    int16_t vfdCursor_y;

    SPISettings spisettings;
    bool nativeFontActive = true;
    bool vfdIgnoreLineWrite = false;

    bool vfdHexMode = true;

    float delayMultiplier = 1.0;

    uint8_t writebuffer[4096];

    uint8_t aligned_width;

    // Serial printing options
    bool raw = true;

    uint8_t defaultPort = 1;
    uint32_t nextprint = 0;
    

    

public:

    uint8_t liveMode;
    void init_pins();
    uint8_t vfdWrite8(uint8_t data);
    void hardReset();

    void printSerial(uint8_t port);

    void printSerial();

    void setFontFromNative(uint8_t index);
    void setFontFromNative(GU_NativeFont_IDX index);

    void getTextBounds(const char *str, int16_t x, int16_t y,
                                 int16_t *x1, int16_t *y1, uint16_t *w,
                                 uint16_t *h);

    void getTextBounds(String str, int16_t x, int16_t y,
                                 int16_t *x1, int16_t *y1, uint16_t *w,
                                 uint16_t *h);

    void getCenteredTextPostion(const char *str, uint8_t *x, uint8_t *y);
    void getCenteredTextPostion(String str, uint8_t *x, uint8_t *y);

};

class GU126X32_K612A4:public GU_K61NA4 {
public:
    GU126X32_K612A4(uint8_t sbusy, uint8_t reset, uint8_t cs, uint8_t din, uint8_t dout, uint8_t clk);

private:
    const int init_pos[2] = {0, 7};
    const GFXfont* init_font = &Noritake5x7;
};
#endif