#include "GU_K61NA4.h"

// NOBUSY will ignore the busy line in waiting to send. 
#define NOBUSY false 

#ifdef DEBUG

  #if defined(USB_SERIAL)
    #define SERIAL_DBG Serial
  #elif defined(USB_DUAL_SERIAL)
    #define SERIAL_DBG Serial
  #elif defined(USB_TRIPLE_SERIAL)
    #define SERIAL_DBG SerialUSB1
  #endif // defined USB_SERIAL
  #define DEBUG_TXT(x)  SERIAL_DBG.print(x)
  #define DEBUG_NUM(x)  SERIAL_DBG.print(x, 10)
  #define DEBUG_OUT(x)  SERIAL_DBG.println(x)
  #define DEBUG_SIG(x)  SERIAL_DBG.println(x)
  #define DEBUG_PRF SERIAL_DBG.printf
#else
  #define DEBUG_TXT(x) 0
  #define DEBUG_NUM(x) 0
  #define DEBUG_OUT(x) 0
  #define DEBUG_SIG(x) 0
  #define DEBUG_PRF(...) ((void)0)
#endif

#ifdef ESP8266
#define DELAY_CLOCK delayMicroseconds(1)
#define DELAY_END_WORD delayMicroseconds(17)
#else
#define DELAY_CLOCK delayMicroseconds(1)
#define DELAY_END_WORD delayMicroseconds(50)
#endif

const GFXfont* GU_NativeFonts[] = {NULL, &Noritake5w, &Noritake5x7, &Noritake10x14};

GU_K61NA4::GU_K61NA4(uint8_t sbusy, uint8_t reset, uint8_t cs,
                     uint8_t din, uint8_t dout, uint8_t clk,
                     uint8_t width, uint8_t height) : GFXcanvas1(width, height)
{
    this->sbusy = sbusy;
    this->reset = reset;
    this->cs = cs;
    this->din = din;
    this->dout = dout;
    this->clk = clk;
    this->liveMode = 1;
    this->resetBounds();

    this->delayMultiplier = 1.0;

    this->nativeFontActive = true;

    Adafruit_GFX::setFont(GU_NativeFonts[2]); // Default is 5x7

    this->aligned_width = ((width + 7) / 8);

    uint32_t bytes = this->aligned_width * height;

    /*
    free(this->buffer);

    this->buffer = writebuffer;
    */

    /*
    if ((this->writebuffer = (uint8_t *)malloc(bytes))) {
      memset(writebuffer, 0, bytes);
    }
    */

    this->defaultPort = 0;

    #ifdef USB_DUAL_SERIAL
        this->defaultPort = 1;
    #endif

    #ifdef USB_TRIPLE_SERIAL
        this->defaultPort = 2;
    #endif

  syncPostInit();
}

void GU_K61NA4::init_pins()
{
    // TODO
    pinMode(reset, OUTPUT);
    pinMode(sbusy, INPUT);
    pinMode(cs, OUTPUT);
    digitalWrite(reset, HIGH);

    spisettings = SPISettings(100000, MSBFIRST, SPI_MODE0);
    SPI.setMISO(din);
    SPI.setMOSI(dout);
    SPI.setSCK(clk);

    SPI.begin();
    digitalWrite(cs, HIGH);
    initialized = true;
}

void GU_K61NA4::syncPostInit()
{
    // Default font on he device is the 5x7
    GFXcanvas1::setFont(init_font);

    // Clear the canvas
    GFXcanvas1::fillScreen(0);
    cursor_x = init_pos[0];
    cursor_y = init_pos[1];
    vfdCursor_x = cursor_x;
    vfdCursor_y = cursor_y;

    if (nativeFontActive) {
      Adafruit_GFX::setFont(GU_NativeFonts[2]); // Default is 5x7
    }
    resetBounds();

    // HexMode defaults to on
    vfdHexMode = true;
}

void GU_K61NA4::vfdInvalidateCursor()
{
  vfdCursor_x = -1;
  vfdCursor_y = -1;
}

uint8_t GU_K61NA4::vfdWrite8(uint8_t data) {
  // TODO
  uint8_t receivedVal = 0;
  /*
  DEBUG_TXT("write: ");
  DEBUG_NUM(data);
  DEBUG_OUT("");
  */
  /*
  DEBUG_TXT("Busy: ");
  DEBUG_NUM(digitalRead(sbusy));
  DEBUG_OUT("");
  */
  SPI.beginTransaction(spisettings);
  digitalWrite(cs, LOW);
  waitdelay();
  receivedVal = SPI.transfer(data);
  delayNanoseconds(500);
  digitalWrite(cs, HIGH);
  /*
  if (receivedVal){
    DEBUG_TXT("recv: ");
    DEBUG_NUM(receivedVal);
    DEBUG_OUT("");
  }
  */
  SPI.endTransaction();
  //delayNanoseconds(100);
  return receivedVal;
}

void GU_K61NA4::hardReset() {
  init_pins();
  digitalWrite(reset, LOW);
  delayNanoseconds(50);
  digitalWrite(reset, HIGH);
  commanddelay(30000);

  // After hard reset the display is empty, sync everything.
  syncPostInit();

  vfdSetWriteMode(true, false, false, 0);
  vfdToggleHexMode();  
}

void GU_K61NA4::setFontFromNative(uint8_t index)
{
  if(index > 0 && index < 4)
  {
    setFont(GU_NativeFonts[index]);
  }
}

void GU_K61NA4::setFontFromNative(GU_NativeFont_IDX index)
{
  setFontFromNative((uint8_t)index - fNoFont);
}

void GU_K61NA4::getCenteredTextPostion(String str, uint8_t *x, uint8_t *y)
{
    getCenteredTextPostion(str.c_str(), x, y);
}

void GU_K61NA4::getCenteredTextPostion(const char *str, uint8_t *x, uint8_t *y)
{
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    DEBUG_PRF("x,y,w,h %d %d %d %d", x1, y1, w, h);
    *x = (WIDTH - w)/2, 
    *y = (HEIGHT + h)/2;

}

void GU_K61NA4::getTextBounds(String str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h){
  return getTextBounds(str.c_str(), x, y, x1, y1, w, h);
}


void GU_K61NA4::getTextBounds(const char *str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
{
  return Adafruit_GFX::getTextBounds(str, x, y, x1, y1, w, h);
  // TODO Figure out why this didn't work

  if(nativeFontActive)
  {
    uint8_t lineheight = 0, charwidth = 0;
    // Compute bounds trivially from number of characters and wrapping for fixed width native fonts
    if (gfxFont == &Noritake10x14){
      lineheight = 14;
      charwidth = 10;
    } else if (gfxFont == &Noritake5x7){
      lineheight = 7;
      charwidth = 5;
    }
    else {
      // For the native variable width font, just use the Adafruit_GFX algo.
      return Adafruit_GFX::getTextBounds(str, x, y, x1, y1, w, h);
    }
    *x1 = x; // Initial position is value passed in
    *y1 = y - lineheight;
    *w = *h = 0; // Initial size is zero

    // check first how big...
    size_t charcount = strlen(str);
    uint8_t lines = 1, lines_open = (HEIGHT - y + lineheight) / (lineheight + 1);
    uint8_t chars_this_line = (WIDTH - x + 1) / (charwidth+1);
    uint8_t chars_full_line = ((WIDTH+1) / (charwidth+1));
    uint8_t more_lines = (charcount - chars_this_line + (chars_full_line - 1)) / chars_full_line;
    if (more_lines >= lines_open) {
      // Use the rest of the display
      *w = WIDTH;
      *h = HEIGHT - (y - lineheight);
      return;
    }
    if (chars_this_line < charcount) {
      *w = charcount * (charwidth + 1) - 1;
      *h = lineheight;
      return;
    }

    // We will line wrap...
    *x1 = 0;
    *w = WIDTH;
    *h = (lineheight + 1) * (more_lines + 1) - 1;
  }
  else
    return Adafruit_GFX::getTextBounds(str, x, y, x1, y1, w, h);
}

void GU_K61NA4::printSerial(uint8_t port)
{
  char pixel_buffer[8];
  Stream *SerialOut = NULL;

  if (port == 0)
    SerialOut = &Serial;

  #ifdef USB_DUAL_SERIAL
  if (port == 1)
    SerialOut = &SerialUSB1;
  #endif

  #ifdef USB_TRIPLE_SERIAL
  if (port == 2)
    SerialOut = &SerialUSB2;
  #endif

  if (!SerialOut)
    return;


  if (!raw){
    SerialOut->printf("%c[H", 0x1b);
    
    for (uint16_t y = 0; y < HEIGHT; y++) {
        for (uint16_t x = 0; x < WIDTH; x++) {
        snprintf(pixel_buffer, 7, "%c", getPixel(x, y)? '#':' ');
        SerialOut->printf(pixel_buffer);
        }
        if (y<HEIGHT-1)
        SerialOut->print("\r\n");
    }
  } else {
    uint16_t fullwidth = ((WIDTH + 7) / 8 ) * 8;
    SerialOut->print("BEGINIMAGE");

    SerialOut->write((uint8_t)(fullwidth>>8));
    SerialOut->write((uint8_t)(fullwidth&0xFF));
    SerialOut->write((uint8_t)(HEIGHT>>8));
    SerialOut->write((uint8_t)(HEIGHT&0xFF));
    memcpy(writebuffer, buffer, (fullwidth / 8) * HEIGHT);
    SerialOut->write(writebuffer, (fullwidth / 8) * HEIGHT);
  }
}

void GU_K61NA4::printSerial()
{
    // Rate limit base call
    if (millis() >= nextprint)
    {
        printSerial(defaultPort);
        nextprint = millis() + 100;
    }
}

void GU_K61NA4::vfdSetCursor(uint8_t x, uint8_t y) {
  vfdWrite8(0x10);
  vfdWrite8(x);
  vfdWrite8(y);
  vfdCursor_x = x;
  vfdCursor_y = y;
  commanddelay();
}

void GU_K61NA4::vfdSetWriteMode(bool horizontal, bool underscore_cursor,
                                bool underscore_flash, uint8_t writetype)
{
  this->horizontal_gfx = !horizontal;
  this->horizontal_cursor = horizontal;
  this->underscore_cursor = underscore_cursor;
  this->underscore_flash = underscore_flash;
  this->writetype = writetype;
  vfdSetWriteMode();
}

void GU_K61NA4::vfdSetWriteMode()
{
  vfdWrite8(0x1a);
  vfdWrite8((!horizontal_gfx    << 7 |
             !horizontal_cursor << 6 |
              cursor_reverse    << 5 |
              underscore_cursor << 4 |
              underscore_flash  << 3 |
              (writetype & 0x03)));
  commanddelay();
}

void GU_K61NA4::vfdSetWriteMode(bool horizontal_gfx, bool horizontal_cursor, bool cursor_reverse, 
                                bool underscore_cursor, bool underscore_flash, uint8_t writetype)
{
  this->horizontal_gfx = horizontal_gfx;
  this->horizontal_cursor = horizontal_cursor;
  this->cursor_reverse = cursor_reverse;
  this->underscore_cursor = underscore_cursor;
  this->underscore_flash = underscore_flash;
  this->writetype = writetype;
  vfdSetWriteMode();
}

void GU_K61NA4::vfdToggleHexMode()
{
  vfdHexMode = !vfdHexMode;
  vfdWrite8(0x1B);
  // send 0x42 to turn off, 0x48 to turn on
  vfdWrite8(0x42 + (0x06 * vfdHexMode));
  commanddelay();
}

void GU_K61NA4::vfdSelectFont(GU_NativeFont_IDX fontcode)
{
  if (fontcode != fNoFont){
    vfdWrite8((uint8_t) fontcode);
  }
}

void GU_K61NA4::vfdWriteCharacter(char c)
{
  if (!liveMode ||
      !nativeFontActive ||
      (c < 0x20 ||
      ((gfxFont == &Noritake10x14 || gfxFont == &Noritake5x7) && c > 0x7F) ||
      ((gfxFont == &Noritake5w) && c > 0x5F))) {
    // Using fixed fonts but character is out of range, don't try
    // Assume vfd is out of sync and set the vfdCursor to an impossible position
    vfdInvalidateCursor();
    vfdIgnoreLineWrite = false;
    return; 
  }
  if(vfdCursor_x != cursor_x || vfdCursor_y != cursor_y) {
    // The native libraries handly line wrapping, so we might be out of sync.
    vfdSetCursor(cursor_x, cursor_y);
  }
  vfdWrite8(c);
  if (vfdHexMode && c == 0x60) {
    // In hex mode, have to send 0x60 twice.
    vfdWrite8(c);
  }
  commanddelay(450);
}


size_t GU_K61NA4::write(uint8_t c)
{
  size_t retval = 0;
  int16_t o_cursor_x = cursor_x, n_cursor_x = -1;
  if(nativeFontActive) {
    // Native fonts blank the area they are writing to.  Simulate this.
    vfdIgnoreLineWrite = true;

    int16_t c_x, c_y, minx, miny, maxx, maxy;
    c_x=minx=maxx=cursor_x;
    c_y=miny=maxy=cursor_y;
    charBounds(c, &c_x, &c_y, &minx, &miny, &maxx, &maxy);
    debug_printf("charbounds: %c, %d, %d, %d, %d, %d, %d\n",
        c, c_x, c_y, minx, miny, maxx, maxy);
    /*
    uint8_t first = pgm_read_byte(&gfxFont->first);
    GFXglyph *glyph = gfxFont->glyph + c - first;
    GFXcanvas1::fillRect(cursor_x, cursor_y, (uint8_t)pgm_read_byte(&glyph->xAdvance)+1,
      -(uint8_t)pgm_read_byte(&gfxFont->yAdvance)-1, 0);
    */

   GFXcanvas1::fillRect(minx, miny, maxx-minx-1, maxy-miny, 0);
    
    vfdIgnoreLineWrite = false;
      
  }

  if(liveMode && nativeFontActive){
    // Pause the line write scripts because we will use the native character writing.
    vfdIgnoreLineWrite = true;
  } 

  // Try to do some smart translation for the tiny font (only have uppercase characters)
  if (nativeFontActive && gfxFont == &Noritake5w){
    if (c >= 0x61 && c <= 0x7a)
      c -= 0x20;
  }

  // GFX write code (which might call any of the line or pixel write codes)
  retval = GFXcanvas1::write(c);

  if(liveMode && nativeFontActive){
    // A line wrap may have occured, check and set cursor accordingly.
    if (cursor_x < o_cursor_x){
      // Line wrap, character should be written at x=0
      o_cursor_x = 0; 
    }
    // Reset to the cursor position before we wrote for the position check.
    n_cursor_x = cursor_x;
    cursor_x = o_cursor_x;
    vfdWriteCharacter(c);

    cursor_x = n_cursor_x;
    vfdCursor_x = cursor_x;

    // Unpause line writing
    vfdIgnoreLineWrite = false;
  }

  return retval;
}


void GU_K61NA4::VFDsetPixel()
{
    vfdWrite8(0x16);
    commanddelay();
}

void GU_K61NA4::VFDclearPixel() {
    vfdWrite8(0x17);
    commanddelay();
}

void GU_K61NA4::commanddelay(uint16_t delay_us)
{
  // Set the delay before the next sip write can be executed.
  //  If actively writing multiple lines, this will block execution in write, 
  //  otherwise it will not force the chip to wait.
  // Check both millis and micros to handle wrap of micros.
  // Default delay for basic commands is 50us
  lastmicros = micros();
  lastmillis = millis();
  nextmicros = lastmicros + (int)(delay_us * delayMultiplier);
  nextmillis = lastmillis + (nextmicros < lastmicros ? 1 : 0); // handle possible overflow of micros
  if (delay_us >= 1000) {
    nextmillis += (delay_us / 1000);
  }  
}

void GU_K61NA4::waitdelay()
{
  // Waits for delay in command

  // First wait for any asserted busy to release.
  do {
    delayNanoseconds(100);
  } while (!digitalRead(sbusy) && !NOBUSY);


  int32_t us_to_wait = nextmicros - micros();
  int32_t ms_to_wait = nextmillis - millis();
  //DEBUG_PRF("Delaying %d ms + %d us (multiplier: %f).\n", ms_to_wait, us_to_wait, delayMultiplier);
  if (ms_to_wait <= 0 && us_to_wait <= 0) {
    return;
  }
  else {
    if (us_to_wait < 0 || us_to_wait >= 16383) {
      // Beyond the delayMicroseconds suggested wait limit, or in overflow, just wait ms.
      delay(ms_to_wait + 1);
    }
    else {
      delayMicroseconds(us_to_wait);
    }
    // Run again to make sure we waited long enough.
    waitdelay();
  }
}

void GU_K61NA4::drawPixel(int16_t x, int16_t y, uint16_t color) {
    GFXcanvas1::drawPixel(x, y, color);
    if (x < WIDTH && y < HEIGHT){
        if (liveMode && !vfdIgnoreLineWrite) {
          vfdSetCursor((uint8_t) x, (uint8_t) y);
          if (color)
              VFDsetPixel();
          else
              VFDclearPixel();
        }
        expandBounds(x, y);
    }
}

void GU_K61NA4::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    expandBounds(x, y, 1, h);
    GFXcanvas1::drawFastVLine(x, y, h, color);
    if (liveMode  && !vfdIgnoreLineWrite) {
        if (color)
            vfdSetOutline(x, y, x, y + h - 1);
        else
            vfdClearOutline(x, y, x, y + h - 1);
    }
    vfdInvalidateCursor();
}

void GU_K61NA4::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    expandBounds(x, y, w, 1);
    GFXcanvas1::drawFastHLine(x, y, w, color);
    if (liveMode  && !vfdIgnoreLineWrite) {
        if (color)
            vfdSetOutline(x, y, x + w -1, y);
        else
            vfdClearOutline(x, y, x + w - 1, y);
    }
    vfdInvalidateCursor();
}

void GU_K61NA4::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    bool oldlivemode = liveMode;
    liveMode = false;
    GFXcanvas1::drawRect(x, y, w, h, color);
    liveMode = oldlivemode;
    expandBounds(x, y, w, h);
    if (liveMode && !vfdIgnoreLineWrite) {
        if (color)
            vfdSetOutline(x, y, x + w - 1, y + h - 1);
        else
            vfdClearOutline(x, y, x + w - 1, y + h - 1);
    }
    vfdInvalidateCursor();
}

void GU_K61NA4::setTextSize(uint8_t s_x, uint8_t s_y)
{
  if (nativeFontActive){
    return;
  }
  Adafruit_GFX::setTextSize(s_x, s_y);
}

void GU_K61NA4::getCharRange(uint8_t *first, uint8_t *last)
{
  *first = pgm_read_byte(&gfxFont->first);
  *last = pgm_read_byte(&gfxFont->last);
}

void GU_K61NA4::display(){
  // Start by aligning write area to byte boundaries, data stored in row0, row1, ... order
  if (maxX >=0){
    if (&writebuffer){
      /*
      // Have memory for the write buffer, so use it.
      // Since data is most likely to be long rather than tall, assume the best 
      // buffer format will be vertical write, horizontal cursor movement.
      // Write 1 byte high at a time but fill with 8 bytes wide when building
      uint8_t byte_height = ((maxY - minY + 7) / 8);
      uint8_t maxY = minY + 8 * byte_height;  // minY + 8, 16, etc.
      */
      minX = minX >= 0 ? minX & 0xF8 : 0;
      minY = minY >= 0 ? minY : 0; 
      maxX = maxX <= WIDTH ? maxX : aligned_width;
      maxY = maxY <= HEIGHT ? maxY : HEIGHT;
      uint8_t xbymin = minX / 8;
      uint8_t xbymax = (maxX + 7) / 8;

      uint8_t write_width = xbymax - xbymin;
      //int16_t startIdx  = (minX + 7) / 8;
      vfdSetWriteMode(true, false, false, 0);
      for (uint8_t i=0; i<write_width; i++) {

        for (uint8_t yp=0; yp<maxY-minY; yp++){
          writebuffer[yp] = buffer[i+aligned_width*(yp+minY)];
        }
        vfdSetCursor(minX+8*i, minY);
        DEBUG_PRF("display: i=%d/%d, p=%d, b=%d\n",
                i, write_width, minX+8*i, maxY - minY + 1);
        vfdGraphicWrite(maxY - minY, writebuffer);
      }
    }
    else {
      minX = minX >=0 ? minX & 0xF8 : 0;
      maxX = (maxX + 7) / 8;
      maxX = maxX <= WIDTH ? maxX: aligned_width;
      maxY = maxY <= HEIGHT ? maxY : HEIGHT;
      uint8_t write_width = (maxX - minX) / 8;
      int16_t startIdx  = (minX + 7) / 8;
      vfdSetWriteMode(true, false, false, 0);
      vfdSetCursor(minX, minY);
      //DEBUG_PRF("display: minX=%d, minY=%d, write_width=%d, startIdx=%d\n",
      //        minX, minY, write_width, startIdx);
      for (uint8_t row=minY; row<maxY; row++){
        //DEBUG_PRF("displayline: (idx: %d)\n", startIdx + row * aligned_width);
        vfdGraphicWrite(write_width, &buffer[startIdx + row * aligned_width]);
      }
    }
  }
  vfdInvalidateCursor();
  resetBounds();
}

void GU_K61NA4::clear(){
  resetBounds();
  fillScreen(0);
  vfdInvalidateCursor();
}


void GU_K61NA4::setFont(const GFXfont *f)
{
  Adafruit_GFX::setFont(f);
  for (int i=fNoFont; i<=fN10x14; i++) {
    if(f == GU_NativeFonts[i-fNoFont]) {
      nativeFontActive = true;
      vfdSelectFont((GU_NativeFont_IDX)i);
      textsize_x = textsize_y = 1;
      return;
    }
  }
  nativeFontActive = false;
}

void GU_K61NA4::setDelayMultiplier(float mult)
{
  delayMultiplier = mult;
}

void GU_K61NA4::setRotation(uint8_t r)
{
  //Refuse to set any rotation
  return;
}

void GU_K61NA4::printCentered(const char *str, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  int16_t x1, y1;
  uint16_t tw, th;
  uint16_t cx, cy;
  getTextBounds(str, 0, 0, &x1, &y1, &tw, &th);
  if (w>WIDTH)
    w = WIDTH;
  if (h>HEIGHT)
    h = HEIGHT;

  if (w==0)
    cx = x;
  else
    cx = (x + w - tw)/2;

  if (h==0)
    cy = y;
  else
    cy = (y + th + h)/2;

  if(x==-1)
    cx = cursor_x;

  if(y==-1)
    cy = cursor_y;

  setCursor(cx, cy);
  print(str);
}

void GU_K61NA4::printCentered(const char *str)
{
  printCentered(str, 0, 0, WIDTH, HEIGHT);
}

void GU_K61NA4::printCentered(String str)
{
  printCentered(const_cast<char *>(str.c_str()));
}

void GU_K61NA4::printCentered(String str, int16_t x, int16_t y, uint16_t w, uint16_t h)
{
  printCentered(const_cast<char *>(str.c_str()), x, y, w, h);
}

void GU_K61NA4::setCursorFlash(bool flash)
{
  underscore_flash = flash;
  underscore_cursor = flash;
  vfdSetWriteMode();
}

void GU_K61NA4::toggleCursorFlash()
{
  underscore_cursor = underscore_flash = !underscore_cursor;
}

void GU_K61NA4::debug_printf(const char *str, ...)
{
  va_list va;
  va_start(va, str);
  int nchars = vsnprintf(0, 0, str, va) + 1;
  va_end(va);
  char *buf = (char*)malloc(nchars);
  va_start(va, str);
  int lenwritten = vsnprintf(buf, nchars, str, va);
  va_end(va);

  DEBUG_PRF(buf);

  free(buf);
}

int GU_K61NA4::printf_r(const char *str, ...)
{
  va_list va;
  va_start(va, str);
  // longest thing we can print is a line anyway, so make a buf the size of the display
  int nchars = vsnprintf(0, 0, str, va) + 1;
  va_end(va);
  char *buf = (char*)malloc(nchars);
  va_start(va, str);
  int lenwritten = vsnprintf(buf, nchars, str, va);
  va_end(va);

  int16_t x1, y1 = cursor_y;
  uint16_t w = WIDTH, h;

  char* buf_to_write = buf;

  getTextBounds(buf_to_write, WIDTH - cursor_x, cursor_y, &x1, &y1, &w, &h);

  while(y1 + h - 1 !=  cursor_y && w > WIDTH - cursor_x)
  {
    // Truncate the text to be written to the amount needed for cursor_x - w >= 0
    getTextBounds(++buf_to_write, WIDTH - cursor_x, cursor_y, &x1, &y1, &w, &h);
  }

  setCursor(cursor_x - w, cursor_y);
  this->print(buf_to_write);
  free(buf);
  return lenwritten;
}

void GU_K61NA4::displayOn(bool on){
  vfdWrite8(0x1b);
  vfdWrite8(on ? 0x50 : 0x46);
}

void GU_K61NA4::setBrightness(uint8_t level){
  // level from 0 - 255, but the display only supports 0XF8 - 0xFF with 0xF8 being off.
  // Any value from 0-32 is off, 1 brightness level per 32. 
  vfdWrite8(0x1b);
  vfdWrite8(0xF8 | level>>5);
  commanddelay();
}

void GU_K61NA4::expandBounds(int16_t x, int16_t y) {
    expandBounds(x, y, 0, 0);
}

void GU_K61NA4::expandBounds(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (w<0) {
    x+=w;
    w*=1;
  }
  if (h<0) {
    y+=h;
    h*=-1;
  }

  // Limit to the known canvas so we don't overflow things.
  w = min(w + x, WIDTH) - x;
  h = min(h + y, HEIGHT) - y;

  minX = constrain(x, -1, maxX);
  maxX = constrain(x + w, minX, WIDTH);
  minY = constrain(y, -1, maxY);
  maxY = constrain(y + h, minY, HEIGHT);
}

void GU_K61NA4::resetBounds()
{
    minX = 1000;
    maxX = -1;
    minY = 1000;
    maxY = -1;
}

void GU_K61NA4::vfdSetArea(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb)
{
  vfdAreaCommand(0x11, xl, yt, xr, yb);
}

void GU_K61NA4::vfdClearArea(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb)
{
  vfdAreaCommand(0x12, xl, yt, xr, yb);
}

void GU_K61NA4::vfdInvertArea(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb)
{
  vfdAreaCommand(0x13, xl, yt, xr, yb);
}

void GU_K61NA4::vfdSetOutline(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb)
{
  vfdAreaCommand(0x14, xl, yt, xr, yb);
}

void GU_K61NA4::vfdClearOutline(uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb)
{
  vfdAreaCommand(0x15, xl, yt, xr, yb);
}

void GU_K61NA4::vfdAreaCommand(uint8_t command, uint8_t xl, uint8_t yt, uint8_t xr, uint8_t yb)
{
  // All area commands have the same 1ms delay, so use one command
  if (xl>xr) {
    uint8_t tmp = xr;
    xr=xl;
    xl=tmp;
  }
  if(yt>yb) {
    uint8_t tmp = yb;
    yb=yt;
    yt=tmp;
  }
  vfdWrite8(command);
  vfdWrite8(xl);
  vfdWrite8(yt);
  vfdWrite8(xr);
  vfdWrite8(yb);
  commanddelay(1250);
  vfdInvalidateCursor();
}

void GU_K61NA4::vfdGraphicWrite(uint8_t len, uint8_t *bytes)
{
  vfdWrite8(0x18);
  vfdWrite8(len);
  for (int i=0; i<len; i++){
    vfdWrite8(bytes[i]);
    commanddelay(250);
  }
  //commanddelay(250 * len + 50);
}

void GU_K61NA4::endWrite()
{
  printSerial();
}

void GU_K61NA4::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if(w<0) {
      x+=w;
      w*=-1;
    }
    if(h<0) {
      y+=h;
      h*=-1;
    }
    bool oldlivemode = liveMode;
    liveMode = false;
    GFXcanvas1::fillRect(x, y, w, h, color);
    liveMode = oldlivemode;
    expandBounds(x, y, w, h);
    if (liveMode && !vfdIgnoreLineWrite) {
        if (color)
            vfdSetArea(x, y, x + w - 1, y + h - 1);
        else
            vfdClearArea(x, y, x + w - 1, y + h - 1);
    }
}

void GU_K61NA4::fillScreen(uint16_t color)
{
    GFXcanvas1::fillScreen(color);
    if (liveMode) {
        if (color)
            vfdSetArea(0, 0, WIDTH, HEIGHT);
        else
            vfdClearArea(0, 0, WIDTH, HEIGHT);
    }
    vfdInvalidateCursor();
}

/**************************************************************************/
/*!
   @brief      Draw a PROGMEM-resident 1-bit image at the specified (x,y)
   position, using the specified foreground (for set bits) and background (unset
   bits) colors.
    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   Width of bitmap in pixels
    @param    h   Height of bitmap in pixels
    @param    color 16-bit 5-6-5 Color to draw pixels with
    @param    bg 16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void GU_K61NA4::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
                              int16_t w, int16_t h, uint16_t color,
                              uint16_t bg) {


  if ((color != 0 && bg != 0) || color == bg){
    // Gotta guess which is white and which black by luma numbers
    // failing that, go with FG=white BG=black
    uint16_t bgluma, fgluma;
    bgluma = (((bg >> 10) & 0x3E) * 218) + (((bg >> 5) & 0x3F) * 732) + (((bg << 1) & 0x3E) * 74);
    fgluma = (((color >> 10) & 0x3E) * 218) + (((color >> 5) & 0x3F) * 732) + (((color << 1) & 0x3E) * 74);
    if(bgluma > fgluma){
      bg=1;
      color=0;
    }
    else{
      bg=0;
      color=1;
    }
  }

  // Update the backing image, and then rely on existing display to write efficiently (including edge of bounds calculations).
  vfdIgnoreLineWrite = true;
  GFXcanvas1::drawBitmap(x, y, bitmap, w, h, color, bg);
  vfdIgnoreLineWrite = false;

  expandBounds(x, y, w, h);

  if (liveMode) {
    display();
    vfdInvalidateCursor();
  }
}

void GU_K61NA4::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w,
                              int16_t h, uint16_t color) {
  
  // Update the backing image, and then rely on existing display to write efficiently (including edge of bounds calculations).
  vfdIgnoreLineWrite = true;
  GFXcanvas1::drawBitmap(x, y, bitmap, w, h, color);
  vfdIgnoreLineWrite = false;

  expandBounds(x, y, w, h);

  if (liveMode) {
    display();
    vfdInvalidateCursor();
  }
}

void GU_K61NA4::drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(x,y,bitmap,w,h,color,!color);
}

GU126X32_K612A4::GU126X32_K612A4(uint8_t sbusy, uint8_t reset, uint8_t cs, uint8_t din,
        uint8_t dout, uint8_t clk) : GU_K61NA4(sbusy, reset, cs, din, dout, clk, 126, 32)
{
}
