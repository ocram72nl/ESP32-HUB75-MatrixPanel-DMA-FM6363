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


//bits of the CFG1 register for FM6363
// Bits   RED     GREEN   BLUE    Name      Description
// 15     0       0       0       OPEN_DET  Open circuit detection:
//                                          0: Disable
//                                          1: Enable
// 14     0       0       0       GCLK_N    This bit controls the number of GCLKs for frames and lines:
//                                          1: 6 GCLKs for the frame header and 78 GCLKs for each subsequent line;
//                                          0: 4 GCLKs for the frame header and 74 GCLKs for each subsequent line;
// 13-08  011111  011111  011111  SCAN_LINE Number of scan lines:
//                                          0: 1 line
//                                          1: 2 lines
//                                          2: 3 lines
//                                          ...
//                                          63: 64 lines
// 07-06  11      10      10      OPT       Low gray dot/high light coupling optimization:
//                                          00: Test mode
//                                          01: Level 2 optimization (recommended value)
//                                          10: Level 3 optimization (optimized high light coupling recommended value)
//                                          11: Test mode
// 05-04  11      11      11      TEST      CFG1<5:4>=01 & CFG3<9:8>=11  ==> Enable REG5<8:0>
//                                          CFG1<5:4>=01                 ==> Enable REG4<3:1>
//                                          CFG1<5:4>=01 & CFG3<9:8>=01  ==> Enable CFG_DBGX configuration (not listed)
// 03     0       0       0                 Cross-board coupling:
//                                          0: Off
//                                          1: On
// 02-00  000     000     000     TEST      Force minimum grayscale output:
//                                          001~111 Output grayscale 1~7 respectively
enum{
  FM6363_CFG1_LC_MASK   = 0x3F00,     // 0b0011111100000000, bits 13-18 (0..63) = line count 1..64 - sync
  FM6363_CFG1_LC_OFFSET = 8,
  FM6363_CFG1_R         = 0x1FF0,   // 32 scan lines
  FM6363_CFG1_G         = 0x1FB0,   // 32 scan lines
  FM6363_CFG1_B         = 0x1FB0,   // 32 scan lines
};

//bits of the CFG2 register for FM6363
// Bits   RED         GREEN       BLUE      Name      Description
// 15     1           1           1         TEST      0: Line scan configuration 33~64 scans (default)
//                                                    1: Line scan configuration 1~64 scans
// 14-10  11100       11001       10101     ADJ       Blanking control register:
//                                                    1-31 levels correspond to register cfg2[14:10]=00000-11111
//                                                    Enable register cfg3[2]
//                                                    Recommended R=31, G=28, B=23
// 09     1           1           1         I_DIV4N   Current gear adjustment
//                                                    1: High current gear
//                                                    0: Low current gear
// 08-01  11001110   11001110   11001110    IGAIN     Constant current source output configuration register,
//                                                    IOUT=19*IGAIN/(Rext*256) @ I_DIV4N=1
//                                                    IOUT=19*IGAIN/(Rext*1024) @ I_DIV4N=0
//                                                    Igain≥64 (required)
// 00     1           1           1                   Blanking mode:
//                                                    0: Fixed blanking (default)
//                                                    1: Pulse blanking
enum{
  FM6363_CFG2_R = 0xF39D,
  FM6363_CFG2_G = 0xE79D,
  FM6363_CFG2_B = 0xD79D,
};

//bits of the CFG3 register for FM6363
// Bits   RED         GREEN       BLUE      Name        Description
// 15     0           0           0         Reserved    --
// 14-12  110         110         110       TEST        Adjusts VDS (for core circuit), range 160mV~520mV
//                                                      From 000~111, the configured VDS level increases in sequence
// 11-10  00          00          00        TEST        PWM output delay
//                                                      Default configuration "00"
// 09-08  00          00          00        TEST        Scan chain configuration
//                                                      Configure CFG1<5:4>=01 before writing
// 07-04  1011        1011        1011      PWM_ALL     Low gray color cast compensation adjustment
//                                                      Levels 1-16 correspond to registers cfg3[7:4]=1111-0000
//                                                      Enable register cfg4[14]
// 03     0           0           0         TEST        Frequency multiplier switch
//                                                      0: Off
//                                                      1: On
// 02     1           1           1         UP_SEL      Void switch:
//                                                      0: Off
//                                                      1: On
// 01-00  01          01          01        VLDO        VLDO voltage
//                                                      4 levels of voltage adjustable
enum{
  FM6363_CFG3_R = 0x60B6,
  FM6363_CFG3_G = 0x60B6,
  FM6363_CFG3_B = 0x60B6,
};

//bits of the CFG4 register for FM6363
// Bits   RED         GREEN       BLUE      Name        Description
// 15     0           0           0         TEST        Blanking Widening 1GCLK
//                                                      0: Off
//                                                      1: On
// 14     1           1           1         PWM_ADD_EN  Low Gray Color Cast Compensation Switch
//                                                      0: Off
//                                                      1: On
// 13     0           0           0         TEST        SDI to SDO Delay Control
//                                                      0: Delay
//                                                      1: No Delay
// 12     1           1           1         Non_clp_en  Coupling Optimization Enable
//                                                      0: Off
//                                                      1: On
// 11-10  10          10          10        TEST        Channel Closing Speed
//                                                      00: Slowest
//                                                      11: Fastest
// 09-08  10          10          10        TEST        Channel Opening Speed
//                                                      00: Slowest
//                                                      11: Fastest
// 07     0           0           0         OPEN_SCAN   Open Data Write Enable
//                                                      0: No Update
//                                                      1: Updateable
// 06     0           1           1         TEST        Clamp Switch
//                                                      0: Off
//                                                      1: On
// 05-04  00          11          11        TEST        Difference between Blanking (V+) and Clamp (V-)
//                                                      The upper two bits of VA
// 03     0           0           0         TEST        Force PWM to output full brightness grayscale
//                                                      0: Off
//                                                      1: On
//                                                      Used with open circuit
// 02     0           0           0         TEST        Black screen energy saving switch
//                                                      0: Off
//                                                      1: On
// 01     0           0           0         VRG_BUF     VRG_BUF bias current
//                                                      0: Default
//                                                      1: Double
// 00     0           0           0         TEST        Display grayscale is halved
//                                                      0: Off (default)
//                                                      1: On
enum{
  FM6363_CFG4_R = 0x5A00,
  FM6363_CFG4_G = 0x5A70,
  FM6363_CFG4_B = 0x5A70,
};

//bits of the CFG5 register for FM6363
// Bits   RED         GREEN       BLUE      Name          Description
// 15     0           0           0         Reserved      --
// 14     1           1           1                       Clamp/Blanking Enhancement
//                                                        0: Disable (default)
//                                                        1: Enable
// 13     1           1           1         Non_clp_v_sel 0: (default)
//                                                        1:
// 12-11  11          11          11                      The difference between blanking (V+) and clamping (V-)
//                                                        The lower two bits of VA
// 10     1           1           1                       Blanking Enhancement
//                                                        0: Disable (default)
//                                                        1: Enable
// 09     1           1           1                       Clamp Enhancement
//                                                        0: Disable (default)
//                                                        1: Enable
// 08     0           0           0                       CFG1~4 Value Reading
//                                                        0: Disable
//                                                        1: Enable
// 07    0            0           0         Reserved      --
// 06-05 00           00          00        TEST          00: SDO output SDI
//                                                        01: SDO output is always 0
//                                                        10: SDO output LDO
//                                                        11: SDO output VGR
// 04-02  010         010         010                     010 Automatic current gear adjustment
//                                                        000/001/111 correspond to the 4 gears of automatic gear shifting
// 01-00  00          00          00                      00/01: Open circuit detection
//                                                        11: Forced open circuit
//                                                        10: Forced normal
enum{
  FM6363_CFG5_R = 0x7E08,
  FM6363_CFG5_G = 0x7E08,
  FM6363_CFG5_B = 0x7E08,
};

//indexes of data arrays for registers for FM6363
enum{
  FM6363_CFG1 = 0,
  FM6363_CFG2,
  FM6363_CFG3,
  FM6363_CFG4,
  FM6363_CFG5,
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
// What are the DMA prefix buffers?
// What are the DMA suffix buffers?
// What is the DSUFFIX count?
enum{FM6363_PREFIX_CNT = 1}; //number of DMA prefix buffers
enum{FM6363_SUFFIX_CNT = 1}; //number of DMA suffix buffers
enum{FM6363_DSUFFIX_CNT = 2}; //number of sets of DMA descriptors per suffix
// Theoretically, with double buffering of DMA output, the more lines, the faster the frame output (less need to wait for cycles to complete regeneration).
// But at the same time there is more memory consumption

enum{FM6363_ROW_OE_CNT = 74}; //number of OE pulses to switch the line
enum{FM6363_ROW_OE_ADD_LEN = 30}; //pause between line pulses OE (for 74 - optional, for 595 - required)
// Curious: Why is GCLK half of DCLK frequency? Should GCLK not be at least 20% faster than DCLK?
enum{FM6363_ROW_OE_LEN = FM6363_ROW_OE_CNT*2 + FM6363_ROW_OE_ADD_LEN}; //total length in ticks per line OE (*2 - rise+fall)

// functions for filling the value of the configuration register for channels 1 and 3
void setDriverReg(driver_reg_t& driver_reg, driver_reg_t value, driver_reg_t mask, uint8_t offset);
void setDriverRegRGB(driver_rgb_t* driver_reg, driver_reg_t value, driver_reg_t mask, uint8_t offset);

#endif
