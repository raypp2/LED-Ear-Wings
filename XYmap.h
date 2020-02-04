// Helper functions for a two-dimensional XY matrix of pixels.
// Special credit to Mark Kriegsman
//
//              This special 'XY' code simulates an 8x11 matrix (square)
//
//              Writing to and reading from the 'holes' in the layout is 
//              also allowed; holes retain their data, it's just not displayed.
//
//              You can also test to see if you're on or off the layout
//              like this
//                if( XY(x,y) > LAST_VISIBLE_LED ) { ...off the layout...}
//
//              X and Y bounds checking is also included, so it is safe
//              to just do this without checking x or y in your code:
//                leds[ XY(x,y) ] == CRGB::Red;
//              All out of bounds coordinates map to the first hidden pixel.
//
//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;


// Params for width and height
const uint8_t kMatrixWidth = 8;
const uint8_t kMatrixHeight = 11;

// Pixel layout
//
//      0  1  2  3  4  5  6  7 
//   +-------------------------
// 0 |  0  1  2  3  4  5  .  . 
// 1 |  6  7  8  9 10  .  .  . 
// 2 | 11 12 13 14 15  .  .  .
// 3 | 16 17 18 19 20  .  .  .
// 4 | 21 22 23 24 25  .  .  .
// 5 | 26 27 28 29 30 31  .  . 
// 6 | 32 33 34 35 36 37 38  . 
// 7 | 39 40 41 42 43 44 45 46 
// 8 | 47 48 49 50 51 52 53  . 
// 9 | 54 55 56 57 58  .  .  . 
// 10| 59 60 61 62  .  .  .  . 

#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds[ NUM_LEDS ];


// This function will return the right 'led index number' for 
// a given set of X and Y coordinates 
// This code, plus the supporting 80-byte table is much smaller 
// and much faster than trying to calculate the pixel ID with code.

#define LAST_VISIBLE_LED 62
uint8_t XY( uint8_t x, uint8_t y)
{
  // any out of bounds address maps to the first hidden pixel
  if( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

  const uint8_t XYTable[] = {
     0,   1,   2,   3,   4,   5,  63,  64,
     6,   7,   8,   9,  10,  65,  66,  67,
    11,  12,  13,  14,  15,  68,  69,  70,
    16,  17,  18,  19,  20,  71,  72,  73,
    21,  22,  23,  24,  25,  74,  75,  76,
    26,  27,  28,  29,  30,  31,  77,  78,
    32,  33,  34,  35,  36,  37,  38,  79,
    39,  40,  41,  42,  43,  44,  45,  46,
    47,  48,  49,  50,  51,  52,  53,  80,
    54,  55,  56,  57,  58,  81,  82,  83,
    59,  60,  61,  62,  84,  85,  86,  87
  };

  uint8_t i = (y * kMatrixWidth) + x;
  uint8_t j = XYTable[i];
  return j;
}
