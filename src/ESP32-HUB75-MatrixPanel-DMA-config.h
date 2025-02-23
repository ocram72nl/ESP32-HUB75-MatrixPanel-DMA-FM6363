#ifndef ESP32_HUB75_MATRIXPANEL_CONFIG_H
#define ESP32_HUB75_MATRIXPANEL_CONFIG_H

#include "ESP32-HUB75-MatrixPanel-DMA-types.h"

//Debug messages via UART
#ifndef DISABLE_SERIAL_DEBUG
  #define SERIAL_DEBUG
#endif

//optimizing video memory and code usage for FM6363
#define USE_COLORx16 //16-bit color for FM6363

#ifndef BRIGHTNESS
 #define BRIGHTNESS 255
#endif
enum{BRIGHTNESS_DEFAULT = BRIGHTNESS};

#ifndef MATRIX_WIDTH
 #define MATRIX_WIDTH                128   // Single panel of 128 pixel width
#endif
enum{PANEL_WIDTH = MATRIX_WIDTH};

#ifndef MATRIX_HEIGHT
 #define MATRIX_HEIGHT               64   // CHANGE THIS VALUE to 64 IF USING 64px HIGH panel(s) with E PIN
#endif
enum{PANEL_HEIGHT = MATRIX_HEIGHT};

#ifndef PANEL_COUNT_WIDTH
 #define PANEL_COUNT_WIDTH                1   // Number of modules chained together, i.e. 4 panels chained result in virtualmatrix 64x4=256 px long
#endif
enum{PANEL_COUNT_X = PANEL_COUNT_WIDTH};

#ifndef PANEL_COUNT_HEIGHT
 #define PANEL_COUNT_HEIGHT            1  // Number of modules chained together, i.e. 4 panels chained result in virtualmatrix 64x4=256 px long
#endif
enum{PANEL_COUNT_Y = PANEL_COUNT_HEIGHT};


#ifndef MATRIX_ROWS_IN_PARALLEL
 #define MATRIX_ROWS_IN_PARALLEL     2    // MWA: Changed from 1 - A 1/32 scan, 64pixels height means two rows in parallel right?
#endif
enum{ROWS_IN_PARALLEL = MATRIX_ROWS_IN_PARALLEL};

#ifndef PIXEL_COLOR_DEPTH
 #define PIXEL_COLOR_DEPTH      COLORx16
#endif

//number of DMA buffer lines for FM6363
//the more, the slightly faster the frame output, but it is expensive in memory, so normal values ​​are: 1..4
//half the number of matrix rows must be a multiple of the number of DMA rows
#ifndef DMA_ROW_BUFF_CNT
#define DMA_ROW_BUFF_CNT 1
#endif
enum{FM6363_DMA_ROW_BUFF_CNT = DMA_ROW_BUFF_CNT};

#ifndef SHIFT_DRIVER
#define SHIFT_DRIVER FM6363
#endif

#ifndef CLK_CLOCK
#define CLK_CLOCK HZ_13M
#endif

#ifndef DOUBLE_BUFF
#define DOUBLE_BUFF false
#endif

#ifndef DOUBLE_DMA_BUFF
#define DOUBLE_DMA_BUFF false
#endif


#ifndef LATCH_BLANK
#define LATCH_BLANK 1
#endif

#ifndef CLK_PHASE
#define CLK_PHASE CLK_POZITIVE
#endif

//for shift
#ifndef LAT_BLANKING
#define LAT_BLANKING 1
#endif

#ifndef R1_PIN
#define R1_PIN 14
#endif
#ifndef G1_PIN
#define G1_PIN 15
#endif
#ifndef B1_PIN
#define B1_PIN 16
#endif
#ifndef R2_PIN
#define R2_PIN 17
#endif
#ifndef G2_PIN
#define G2_PIN 18
#endif
#ifndef B2_PIN
#define B2_PIN 19
#endif
#ifndef A_PIN
#define A_PIN 21
#endif
#ifndef B_PIN
#define B_PIN 22
#endif
#ifndef C_PIN
#define C_PIN 23
#endif
#ifndef D_PIN
#define D_PIN 25
#endif
#ifndef E_PIN
#define E_PIN 26
#endif
#ifndef LAT_PIN
#define LAT_PIN 27
#endif
#ifndef OE_PIN
#define OE_PIN 5
#endif
#ifndef CLK_PIN
#define CLK_PIN 2
#endif
#ifdef ADD_VSYNC_OUT
#ifndef VSYNC_PIN
#define VSYNC_PIN 22
#endif
#endif
#ifdef ADD_HSYNC_OUT
#ifndef HSYNC_PIN
#define HSYNC_PIN 22
#endif
#endif
#ifndef DECODER_INT595
#define DECODER_INT595 false
#endif


// Panel Upper half RGB (numbering according to order in DMA gpio_bus configuration)
enum{
  BITS_RGB1_OFFSET = 0, // Start point of RGB_X1 bits
  BIT_R1 = (1<<0),
  BIT_G1 = (1<<1),
  BIT_B1 = (1<<2),

// Panel Lower half RGB
  BITS_RGB2_OFFSET = 3, // Start point of RGB_X2 bits
  BIT_R2 = (1<<3),
  BIT_G2 = (1<<4),
  BIT_B2 = (1<<5),

// Panel Control Signals
  BIT_LAT = (1<<6),
  BIT_OE = (1<<7),

// Panel GPIO Pin Addresses (A, B, C, D etc..)
  BITS_ADDR_OFFSET = 8,  // Start point of address bits
  BIT_A = (1<<8),
  BIT_B = (1<<9),
  BIT_C = (1<<10),
  BIT_D = (1<<11),
  BIT_E = (1<<12),
  #ifdef ADD_HSYNC_OUT
  BIT_HSYNC = (1<<14),
  #endif
  #ifdef ADD_VSYNC_OUT
  BIT_VSYNC = (1<<15),
  #endif

  BITMASK_ADDR  = BIT_A + BIT_B + BIT_C + BIT_D + BIT_E,  // bit mask of the line address
  BITMASK_RGB1  = BIT_R1 + BIT_G1 + BIT_B1,               // bit mask of the color data of the top lines
  BITMASK_RGB2  = BIT_R2 + BIT_G2 + BIT_B2,               // bit mask of the color data of the bottom lines
  BITMASK_RGB12 = BITMASK_RGB1 + BITMASK_RGB2,            // Total color bitmask
};

// MWA: Don't need this as we have 64pix height panels, so we can just use address lines A-E.
// For control card, SDI is the C of    3-8 decoder, DCK is the A of    3-8 decoder, RCK    is the B of    3-8 decoder
enum{BIT_SDI =  BIT_C};
enum{BIT_DTK = BIT_A};
enum{BIT_RCK = BIT_B};

extern const hub75_cfg_t hub75_cfg_default;

#endif
