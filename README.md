# GU-K612A4 modules
Implementation of the base class of the Adafruit_GFX library for the Noritake Itron GU-K612A4 VFD, specifically built for the GU126x32F-K612A4 module.

## It works!
![Display connected to Arduino](images/connected.JPG)


![Display close-up](images/zoomed_in.JPG)

## Connecting the display

This code is intended to work with the hardware SPI code, which typically restricts which port may be used for SCLK, DOUT, and DIN on the Arduino side.  Select appropriately.  Additionally, this module expects to be connected to the /SS, /RESET, and /MB lines on the VFD (the /MB line taken from the serial port connector).  Those may be any arbitrary digital out (in for /MB) line.

The Arduino pins used are defined in the constructor for the class. The pins here are as in the examples.

## Update modes.
The interface to the display, even in synhronous mode, is very slow. The library can be used in three ways, depending on the desired performance and coding complexity required.
There are examples for each one.

### Live mode

In this mode (selected by setting *vfd*`.livemode` to 1), the screen is updated as you draw. This is immensely slow, as a whole byte is transferred per pixel written. Since none of the drawing primitives of Adafruit_GFX have been optimised, things like font drawing and rectangle fills proceed one pixel at a time.

Expansions from the original code for live mode have been made to include sending native commands whenever possible to match the display.  Any commands which cannot be replicated by native commands are first performed on the memory backed GFXCanvas1 and then written using slower bitmapping to the VFD  (Full screen write may take +100 ms)

### Clear changed mode
By setting *vfd*`.livemode` to 0, drawing primitives only draw to the backing store, not the display. The display is only updated when *vfd*`.display()` is called. In order to reduce the amount of data transferred, only the pixel columns changed in the previous draw calls are copied to the display. If you draw a rectangle at 10,10, size 5x5, only pixel columns 10 to 15 get updated. This is the fastest mode for update, but it does require that you manage the blanking of regions overwritten by text that use the bitmap fonts.

## Clear and redraw mode
Set *vfd*`.livemode` to 0, then whenever the display should change, clear the backing store (with *vfd*`.clear()`), redraw all of the contents, then update the display with *vfd*`.display()`. You won't have to worry about blanking areas that need to change, and of course no flickernig happens as only the backing store is cleared. Updating the display is so slow that the computational overhead of regenerating the entire didplay contents is usually minimal. This is rather like writing display code for the U8G2 library.

## Details
More info [Here](Details.md)

## Acknowledgements
The small font in the static demo is copied from https://github.com/robjen/GFX_fonts, as the library is not yet in the Arduino Library manager. And 140 pixels isn't long enough for a Github URL!

Built on the library from shufflebits https://github.com/shufflebits/GU140X32F-7703A designed to work with a simlar module.

## TODO
The K612A4 series VFDs are also capable of addressing 8 pins of digital I/O onboard, which could be implemented.  Some speed improvements may also be possible (as wired for testing, this was not able to break 100 kHz for SPI speed, which is far less than the spec sheet claims the VFD to be capable of).