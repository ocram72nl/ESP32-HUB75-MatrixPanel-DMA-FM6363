#ifndef _ESP32_HUB75_MATRIXPANEL_DMA_LEDDRIVERS
#define _ESP32_HUB75_MATRIXPANEL_DMA_LEDDRIVERS

#include "stdbool.h"
#include "stdint.h"

//size of driver registers
// MWA: Changed from "int16_t" to "uint16_t" as it was causing conversion error
// THIS IS AN ISSUE! Probably really needs to be int16_t! How to fix this?
typedef uint16_t driver_reg_t;
//bit number of registers
enum{DRIVER_BITS = sizeof(driver_reg_t)*8};
//enum{DRIVER_BITS = 16};

//data structure of configuration registers for each color channel
typedef struct
{
 driver_reg_t r;
 driver_reg_t g;
 driver_reg_t b;
} driver_rgb_t;

//------------------------------------------------ Optional ---------------------------------------------------
//delay between commands (for control through a logic analyzer - optional)
//enum{FM6363_CMD_DELAY = 16};
enum{FM6363_CMD_DELAY = 2};
//adding a zone after each substring (for control through a logical analyzer - optional)
//enum{SUBROW_ADD_LEN = 8};
enum{SUBROW_ADD_LEN = 0};
//adding a zone after each line (for control through a logical analyzer - optional)
//enum{ROW_ADD_LEN = 8};
enum{ROW_ADD_LEN = 0};
//adding a zone after each frame (for control through a logic analyzer - optional)
//enum{FRAME_ADD_LEN = 16};
enum{FRAME_ADD_LEN = 0};

//lat duration parameters for SHIFT drivers
//enum{MAX_LAT_BLANKING = 4};
//enum{DEFAULT_LAT_BLANKING = 1};
enum{MAX_LAT_BLANKING = 4};
enum{DEFAULT_LAT_BLANKING = 1};

//------------------------------------ do not change (except register values) ---------------------------------------

//bit number of line address
enum{ROW_ADDR_BITS = 5};

//------------------------------------ FM6363 ---------------------------------------
//command codes for FM6363 (LAT length)
enum{
  FM6363_DATA_LATCH = 1,
  FM6363_WR_DBG = 2,     //PRE_ACT +
  FM6363_V_SYNC = 3,
  FM6363_WR_CFG1 = 4,    //PRE_ACT +
  FM6363_RD_CFG1 = 5,
  FM6363_WR_CFG2 = 6,    //PRE_ACT +
  FM6363_RD_CFG2 = 7,
  FM6363_WR_CFG3 = 8,    //PRE_ACT +
  FM6363_RD_CFG3 = 9,
  FM6363_WR_CFG4 = 10,   //PRE_ACT +
  FM6363_RD_CFG4 = 11,
  FM6363_EN_OP = 12,     //PRE_ACT +
  FM6363_DIS_OP = 13,
  FM6363_PRE_ACT = 14,
};

//bits of the DBG register for FM6363 taken from wata-net
// No further information
enum{
  FM6363_DBG_x = 0x7E08,
};

//bits of the CFG1 register for FM6363, taken from wata-net
// Bits   RED     GREEN   BLUE    Description
// 15-14  00      00      00      Unknown
// 13-08  011111  011111  011111  Number of scan lines. 1 (2 lines) - 63 (64 lines)
// 07-06  11      10      10      Enhance log-grayscale uniformity level
// 05-04  11      11      11      Unknown
// 03     0       0       0       Cross-board coupling Optimization enable
// 02-00  000     000     000     Unknown
enum{
  FM6363_CFG1_LC_MASK   = 0x3F00,     // 0b0011111100000000, bits 13-18 (0..63) = line count 1..64 - sync
  FM6363_CFG1_LC_OFFSET = 8,
  FM6363_CFG1_R         = 0x1FF0,   // 32 scan lines
  FM6363_CFG1_G         = 0x1FB0,   // 32 scan lines
  FM6363_CFG1_B         = 0x1FB0,   // 32 scan lines
};

//bits of the CFG2 register for FM6363, taken from wata-net
// Bits   RED         GREEN       BLUE      Description
// 15     1           1           1         Unknown
// 14-10  11100       11001       10101     The ghost to eliminate level
// 09-01  111001110   111001110   111001110 Current Gain 0x40(12,5%)~1FF(199,22%). Default 0x1CE(160,94%)
// 00     1           1           1         Unknown
enum{
  FM6363_CFG2_R = 0xF39D,
  FM6363_CFG2_G = 0xE79D,
  FM6363_CFG2_B = 0xD79D,
};

//bits of the CFG3 register for FM6363, taken from wata-net
// Bits   RED         GREEN       BLUE      Description
// 15-08  01100000    01100000    01100000  Unknown
// 07-04  1011        1011        1011      Low-grayscale and color cast compensation level
// 03     0           0           0         Unknown
// 02     1           1           1         The ghost to Eliminate enable
// 01-00  01          01          01        Unknown
enum{
  FM6363_CFG3_R = 0x60B6,
  FM6363_CFG3_G = 0x60B6,
  FM6363_CFG3_B = 0x60B6,
};

//bits of the CFG4 register for FM6363, taken from wata-net
// Bits   RED         GREEN       BLUE      Description
// 15     0           0           0         Unknown
// 14     1           1           1         Low-grayscale and color cast compensation enable
// 13     0           0           0         Unknown
// 12     1           1           1         Coupling Optimization Enable
// 11-06  000000      101001      101001    Unknown
// 05-04  00          11          11        Darker compensation of first scan level
// 03-00  0000        0000        0000      Unknown
enum{
  FM6363_CFG4_R = 0x5A00,
  FM6363_CFG4_G = 0x5A70,
  FM6363_CFG4_B = 0x5A70,
};

//indexes of data arrays for registers for FM6363
enum{
  FM6363_CFG1 = 0,
  FM6363_CFG2,
  FM6363_CFG3,
  FM6363_CFG4,
  FM6363_DBG,
  FM6363_REG_CNT
};
//array of commands for writing to registers
extern const uint8_t FM6363_REG_CMD[FM6363_REG_CNT];
//array of values ​​for commands to write to registers
extern const driver_rgb_t FM6363_REG_VALUE[FM6363_REG_CNT];


//length of the start of the vertical synchronization prefix and configuration register update for FM6363
enum{
  FM6363_VSYNC_LEN = (( FM6363_CMD_DELAY  +
                        FM6363_PRE_ACT    +
                        FM6363_CMD_DELAY  +
                        FM6363_EN_OP      +
                        FM6363_CMD_DELAY  +
                        FM6363_V_SYNC     +
                        FM6363_CMD_DELAY  + 1)/2)*2,
  FM6363_PREFIX_START_LEN = ((FM6363_VSYNC_LEN + FM6363_PRE_ACT + FM6363_CMD_DELAY + 1)/2)*2,
};

//transition descriptor indices for FM6363
enum{
  FM6363_EXT_PREFIX = 0,
  FM6363_EXT_DATA,
  FM6363_DESCEXT_CNT,
};

//named constants for FM6363
enum{FM6363_PREFIX_CNT = 1}; //number of DMA prefix buffers
enum{FM6363_SUFFIX_CNT = 1}; //number of DMA suffix buffers
enum{FM6363_DSUFFIX_CNT = 2}; //number of sets of DMA descriptors per suffix
// Theoretically, with double buffering of DMA output, the more lines, the faster the frame output (less need to wait for cycles to complete regeneration).
// But at the same time there is more memory consumption

enum{FM6363_ROW_OE_CNT = 74}; //number of OE pulses to switch the line
enum{FM6363_ROW_OE_ADD_LEN = 30}; //pause between line pulses OE (for 74 - optional, for 595 - required)
enum{FM6363_ROW_OE_LEN = FM6363_ROW_OE_CNT*2 + FM6363_ROW_OE_ADD_LEN}; //total length in ticks per line OE (*2 - rise+fall)

// functions for filling the value of the configuration register for channels 1 and 3
void setDriverReg(driver_reg_t& driver_reg, driver_reg_t value, driver_reg_t mask, uint8_t offset);
void setDriverRegRGB(driver_rgb_t* driver_reg, driver_reg_t value, driver_reg_t mask, uint8_t offset);

#endif
