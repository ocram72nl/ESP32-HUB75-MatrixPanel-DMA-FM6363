#ifndef ESP32_HUB75_MATRIXPANEL_DMA_FM6363C_H
#define ESP32_HUB75_MATRIXPANEL_DMA_FM6363C_H

#include "ESP32-HUB75-MatrixPanel-DMA-config.h"
#include "ESP32-HUB75-MatrixPanel-DMA-leddrivers.h"
#include "color_convert.h"

//#define NO_FAST_FUNCTIONS

/***************************************************************************************/
/* Library Includes!                                                                   */
//#include <memory>
#include "esp_heap_caps.h"
#include "esp32_i2s_parallel_v2.h"

#if defined(USE_GFX_ROOT)
	#include <FastLED.h>
	#include "GFX.h" // Adafruit GFX core class -> https://github.com/mrfaptastic/GFX_Root
#elif !defined(NO_GFX)
  #include "Adafruit_GFX.h" // Adafruit class with all the other stuff
#endif

enum{
  _VIRTUAL_BIT_TOP_DOWN = 1<<0,
  _VIRTUAL_BIT_SERPENTINE = 1<<1,
};

typedef enum{
  VIRTUAL_TOP_DOWN = _VIRTUAL_BIT_TOP_DOWN,
  VIRTUAL_BOTTOM_UP = 0,
  VIRTUAL_S_TOP_DOWN = _VIRTUAL_BIT_SERPENTINE + _VIRTUAL_BIT_TOP_DOWN,
  VIRTUAL_S_BOTTOM_UP = _VIRTUAL_BIT_SERPENTINE,
}virtual_matrix_t;

//output buffer definition
typedef struct{
  ESP32_I2S_DMA_STORAGE_TYPE** rowBits; // for FM6363c - portions of data for output (for 240x120 - you have to save memory)
  size_t row_data_len;                  // buffer length of one DMA line
  size_t frame_prefix_len;              // prefix buffer length
  size_t frame_suffix_len;              // suffix buffer length
  uint8_t all_row_data_cnt;             // total number of DMA line buffers
  uint8_t row_data_cnt;                 // number of DMA lines per frame
  uint8_t frame_prefix_cnt;             // number of frame prefixes: 0,1
  uint8_t frame_suffix_cnt;             // number of frame suffixes: 0,1,2
  int8_t color_bits;                    // color depth per channel
}frameStruct_t;

//constants for the primary frame buffer
//for a 16-bit buffer, the bits should be 16 or 32
typedef uint32_t vbuffer_t;
enum{
  VB_SIZE = sizeof(vbuffer_t),
  VB_MBITS = 5,
  VB_MASK = 31,
};

//defining the primary frame buffer
typedef struct{
  vbuffer_t* frameBits[2];              // buffer data
  size_t len;                           // size of one buffer
  size_t row_len;                       // length of the row in vbuffer_t words
  size_t subframe_len;                  // bit frame size
  uint8_t subframe_cnt;                 // number of bit frames
}frame_buffer_t;

//color in R8G8B8
typedef struct{
  uint8_t  b;
  uint8_t  g;
  uint8_t  r;
}rgb888_t;

//color conversion function
//typedef void(*palleteRGB_p)(uint8_t color, rgb888_t& data_rgb);

//custom procedure for issuing pixel color upon request of forming a DMA line
typedef void(*getUserRGB_p)(int16_t offset_y, int16_t offset_x, rgb888_t& pixel888_high, rgb888_t& pixel888_low);
//custom pixel drawing procedure
typedef void(*drawUserPixel_p)(int16_t x1, int16_t y1, const rgb888_t& color);

/***************************************************************************************/
#ifdef USE_GFX_ROOT
class MatrixPanel_DMA : public GFX {
#elif !defined NO_GFX
class MatrixPanel_DMA : public Adafruit_GFX {
#else
class MatrixPanel_DMA {
#endif

  // ------- PUBLIC -------
  public:
    // MatrixPanel_DMA
    // default predefined values are used for matrix configuraton
    // parameter for the virtual panel (the number of panels in the column is greater than 1)
    // VIRTUAL_S_TOP_DOWN - the beginning of the chain of panels is at the top, the rows of panels are connected by a sepantine
    // VIRTUAL_S_BOTTOM_UP - the beginning of the chain of panels at the bottom, the rows of panels are connected by a sepantine
    // VIRTUAL_TOP_DOWN - the beginning of the chain of panels is at the top, the rows of panels are directed in one direction
    // VIRTUAL_BOTTOM_UP - the beginning of the chain of panels at the bottom, the rows of panels are directed in one direction
    MatrixPanel_DMA(virtual_matrix_t _virtual_panel = VIRTUAL_S_TOP_DOWN);
    // MatrixPanel_DMA
    // @param  hub75_i2s_cfg_t& opts : structure with matrix configuration
    //
    MatrixPanel_DMA(const hub75_cfg_t& opts, virtual_matrix_t _virtual_panel = VIRTUAL_S_TOP_DOWN);

    // Propagate the DMA pin configuration, allocate DMA buffs and start data ouput, initialy blank
    bool begin();
    // overload for compatibility
    bool begin(hub75_pins_t* hub75_pins_ptr);

    //set rotate write to display
    //rotate: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270
    void setRotate(rotate_t _rotate);

    // Mirror display
    void setMirrorX(bool _mirror_x);
    void setMirrorY(bool _mirror_y);

    // Adafruit's BASIC DRAW API (565 colour format)
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);   // overwrite adafruit implementation
    void drawPixelRGB888(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b);
	  virtual void fillScreen(uint16_t color);                        // overwrite adafruit implementation
    void fillScreenRGB888(uint8_t r, uint8_t g, uint8_t b);
    virtual void clearScreen();                                     //clear screen and buffers
    virtual void setColor(uint16_t color);
    virtual void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setTextColorRGB(uint8_t r, uint8_t g, uint8_t b);
    //draw a smooth line with halftones
    void writeLineAA(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    //draw a smooth line with halftones and double thickness for 45 degree lines
    void writeLineAA2(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

    #ifdef USE_GFX_ROOT
	  // 24bpp FASTLED CRGB colour struct support
	  void fillScreen(CRGB color);
    void drawPixel(int16_t x, int16_t y, CRGB color);
    #endif

    #ifndef NO_FAST_FUNCTIONS
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h);
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint8_t r, uint8_t g, uint8_t b);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint8_t r, uint8_t g, uint8_t b);
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h);
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t r, uint8_t g, uint8_t b);
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h);
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t r, uint8_t g, uint8_t b);
    #endif

    void drawIcon (int* ico, int16_t x, int16_t y, int16_t cols, int16_t rows);

    //buffer switching
    void flipBuffer();
    //for compatibility
    void flipDMABuffer();

    //this is just a wrapper to control brightness
    //with an 8-bit value (0-255), very popular in FastLED-based sketches :)
    //@param uint8_t b - 8-bit brightness value
    //!!! for FM6363c you will need to redraw the frame using sendFrame(), or flipBuffer()
    void setPanelBrightness(int b);
    void setBrightness8(uint8_t b);

    void invertDisplay(bool negative);

    //Get a class configuration struct
    const hub75_cfg_t& getCfg() const;

    //Stop the ESP32 DMA Engine. Screen will forever be black until next ESP reboot.
    void stopDMAoutput();

    //Functions for FM6363c

    //send a frame from the video buffer (double buffering will send the output buffer)
    //waitSend - wait for the frame to be output via DMA
    //autoVsync - switch matrix drivers to the sent frame
    void sendFrame(bool waitSend = false, bool autoVsync = true);
    //switch the matrix driver to the sent frame by sending one of the configuration registers
    void sendVsync();
    //turn off matrix driver output
    void panelShowOn();
    //enable matrix driver output
    void panelShowOff();
    //wait for dma to be ready to send data again
    void waitDmaReady();
    //void setpalleteRGB(palleteRGB_p palleteRGB);


    //bool autoShowFrame = true; //auto-output of the sent video buffer

    //procedure for DMA interrupt handler
    void sendCallback();
  // ------- PROTECTED -------
  // those might be useful for child classes, like VirtualMatrixPanel
  protected:
    bool FM6363_clear;         //flag for clearing screen drivers without clearing frame buffers
    hub75_cfg_t m_cfg;          // Matrix i2s settings
    //uint16_t chain_length_x;  //number of panels by X
    //uint16_t chain_length_y;  //number of panels by Y
    uint16_t pixels_per_row;    //total line length in pixels
    uint16_t CurColor;          //current color rgb565
    rgb888_t CurRGB;            //current color R8G8B8
    int16_t rows_send_cnt;      //counter of unsent frame lines
    //uint8_t virtual_panel; //virtual panel
    uint8_t virtual_draw;
    bool show_mirror_x = false;
    bool show_mirror_y = false;
    bool change_xy = false;
    bool mirror_x;
    bool mirror_y;
    bool hw_mirror_x = false;
    bool hw_mirror_y = false;
    bool serpentine_chain; // Are we chained? Ain't no party like a...
    bool top_down_chain;

    //clearing the video buffer (including the screen for the current output buffer)
    //_buff_id - buffer number:
    //0 - displayed, this also clears the screen drivers
    //1 - rendering (with double buffering disabled for FM6363 remains relevant, since these drivers have their own buffer)
    void clearBuffer(uint8_t _buff_id = 1);
    //clear DMA buffer
    //_buff_id - DMA buffer number
    void clearDmaBuffer(uint8_t _buff_id = 0);
    //switching DMA to sending prefix with synchronization
    void sendCBVsync();
    //switch DMA to send string
    void sendCBRow(uint8_t buff_id);
    //draw a filled rectangle
    void fillRectBuffer(int16_t x, int16_t y, int16_t w, int16_t h);
    void fillRectBufferVirtual(int16_t x, int16_t y, int16_t w, int16_t h);
    //draw a point with conditional coordinate exchange
    void _steepDrawPixelRGB(bool steep, int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b);
    void _writeLineAARGB(bool steep, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t r, uint8_t g, uint8_t b);

   // ------- PRIVATE -------
  private:
    //омновные буфера
    uint8_t  rows_per_frame;      //number of lines along the address lines of the matrix module
    frameStruct_t dma_buff;       //video buffer\output buffer for FM6363
    frame_buffer_t frame_buffer;  //primary video buffer for FM6363
    lldesc_t* dmadesc_data[2];    //descriptors of video buffers\output buffers for FM6363
    lldesc_t* dmadesc_prefix;     //prefix handle for FM6363 for driver register management and vertical synchronization (line counter reset)
    lldesc_t* dmadesc_suffix[2];  //screen regeneration descriptors for FM6363 driver registers
    lldesc_t* dmadesc_ext;        //transition descriptors to suffixes for FM6363 - ensure continuous screen regeneration
    uint8_t  cur_suffix_id;
    uint16_t prefix_to_suffix;    //suffix index after prefix
    uint16_t data_to_suffix;      //suffix index after string data
    uint16_t desc_data_cnt;       //number of descriptors for these lines for each video buffer
    uint16_t desc_prefix_cnt;     //number of descriptors for the personnel prefix
    uint16_t desc_suffix_cnt;     //number of frame suffix descriptors per suffix

    //to initialize drivers
    uint16_t driver_cnt;          //number of driver registers (calculation optimization);
    driver_rgb_t* driver_reg;     //copy of configuration registers for drivers (initializing the number of lines, brightness, etc.)
    uint8_t driver_cur_reg;       //index of the current configuration register

    int dma_int_cnt;              //DMA interrupt counter - needed as a crutch for counting DMA transfer interrupts through the OS
    bool bufferReady;             //ready of the video buffer (not delayed and the end of sending the DMA buffer was interrupted)
    bool bufferBusy;              //video buffer busy (prohibiting setting the bufferReady flag and changing the video buffer)


    // Other private variables
    bool initialized;             //display ready
    uint8_t back_buffer_id;       //current secondary buffer for double buffering\current suffix for ICN2038 MWA: DO WE NEED THIS FOR FM6363?
    uint8_t brightness;           //screen brightness from 0 to 255 (0..100%) (in principle, the variable is not needed if you do not set the brightness before initializing the buffers)
    uint16_t* brightness_table;   //gamma\brightness table for converting the brightness of RGB channels

    //output the image in negative
    bool negative_panel;
    //palleteRGB_p userPalleteRGB = NULL;

    //fast roll shift in Y for FM6363 - not fully developed ()
    int32_t scroll_y;
    //fast roll shift in X for FM6363 - only for a straight chain of panels
    int32_t scroll_x;

    //custom procedure for issuing pixel color upon request of forming a DMA line
    getUserRGB_p getUserRGB;
    //custom pixel drawing procedure
    drawUserPixel_p drawUserPixel;
    //automatic image switching after sending video strings to FM6363 drivers
    //otherwise you need to manually call sendVsync(), in which case you can display a pre-sent image with minimal delay
    bool FM6363_auto_vsync;

    //----- general functions -----
    //clear allocated memory
    void buffersFree();
    //memory allocation
    bool allocateDMAmemory();
    //initializing buffers and setting up DMA
    void configureDMA();

    //----- FM6363 functions -----
    //initialize output buffers
    void FM6363initBuffers();
    //primary initialization of screen drivers (sending all configuration registers)
    void FM6363init();
    //filling the prefix with configuration register data
    void FM6363setReg(uint8_t reg_idx, driver_rgb_t* regs_data);
    //turn on/off the screen and frame changing in the prefix
    void FM6363setVSync(bool leds_enable, bool vsync);

    //setting up a custom procedure for issuing pixel color upon request for generating a DMA line
    void setDMAGetUserRGB(getUserRGB_p _getUserRGB);
    //custom pixel drawing procedure
    void setDrawUserPixel(drawUserPixel_p _drawUserPixel);

    //draw a rectangle in the video buffer
    void fillRectFrameBuffer(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

    #ifdef USE_COLORx16
    //get pixel color from video buffer
    void getRGBColor16(int offset_y, int offset_x, rgb888_t& rgb888);
    #endif
    //filling the DMA string buffer
    void prepareDmaRows(uint8_t row_offset, uint8_t dma_buff_id);
}; // end Class header

#endif
