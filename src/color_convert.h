#ifndef _COLOR_CONVERT
#define _COLOR_CONVERT

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
  COLORx1 = 1,      //2 colors via palet (only for FM6363c)
  COLORx2 = 2,      //4 colors via palet (only for FM6363c)
  COLOR111 = 3,     //8 colors 3x1 bitplanes
  COLORx3 = 3,      //8 colors 3x1 bitplanes
  COLORx4 = 4,      //16 colors via palet (only for FM6363c)
  COLOR222 = 6,     //64 colors
  COLORx8 = 8,      //256 colors via palet (only for FM6363c)
  COLOR333 = 9,     //512 colors
  COLOR444 = 12,    //4096 colors
  COLOR555 = 15,    //32768 colors
  COLORx16 = 16,    //HICOLOR (only for FM6363c)
  COLOR565 = 16,    //HICOLOR (only for FM6363c)
  COLOR666 = 18,
  COLOR777 = 21,
  COLORx24 = 24,    //TRUECOLOR 3x8 bitplanes
  COLOR888 = 24,    //TRUECOLOR 3x8 bitplanes
  //COLORx48 = 48,    //in theory DEEPCOLOR 3x16 bitplanes (only for FM6363c)
}color_depth_t;

#define color888to565(r,g,b) color565(r,g,b)

//extern uint16_t Translate8To16Bit[256];
extern const uint8_t lumConvTab[256];
enum{BRIGHT_TABLE_SIZE = sizeof(lumConvTab)};

// Converts RGB111 to RGB565
uint16_t color111to565(uint8_t r, uint8_t g, uint8_t b);
// Converts RGB222 to RGB565
uint16_t color222to565(uint8_t r, uint8_t g, uint8_t b);
// Converts RGB333 to RGB565
uint16_t color333to565(uint8_t r, uint8_t g, uint8_t b);
// Converts RGB444 to RGB565
uint16_t color444to565(uint8_t r, uint8_t g, uint8_t b);
// Converts RGB555 to RGB565
uint16_t color555to565(uint8_t r, uint8_t g, uint8_t b);
// Converts R6G6B6 to RGB565
uint16_t color666to565(uint8_t r, uint8_t g, uint8_t b);
// Converts R7G7B7 to RGB565
uint16_t color777to565(uint8_t r, uint8_t g, uint8_t b);
// Converts R8G8B8 to RGB565
uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
// Converts RXGXBX to RGB565
uint16_t colorXXXto565(uint8_t r, uint8_t g, uint8_t b, uint8_t X);
// Converts RGB565 to RGB888
void color565to888(const uint16_t color, uint8_t &r, uint8_t &g, uint8_t &b);

// Converts packet ColorXXX to RXGXBX
void colorXXXtoRXGXBX(uint16_t colorXXX, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t X);
// Converts RXGXBX to packet ColorXXX
uint16_t colorRXGXBXtoXXX(uint8_t r, uint8_t g, uint8_t b, uint8_t X);



#endif

